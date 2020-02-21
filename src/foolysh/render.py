"""
SDL2 TextureSpriteRenderSystem
"""

from collections import namedtuple
from typing import Dict
from typing import Tuple

import sdl2.ext
from sdl2 import rect
from sdl2 import render

from .tools import common
from .scene import node
from .tools import aabb
from .tools import spriteloader
from .tools import vector2

__author__ = 'Tiziano Bettio'
__license__ = 'MIT'
__version__ = '0.1'
__copyright__ = """Copyright (c) 2020 Tiziano Bettio

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE."""


Sprite = namedtuple('Sprite', ['scale', 'sprite', 'index', 'name'])


class HWRenderer(sdl2.ext.TextureSpriteRenderSystem):
    """
    Provides rendering of the scenegraph.
    """
    def __init__(self, window):
        super(HWRenderer, self).__init__(window)
        self._window = window
        self.renderer = self.sdlrenderer
        self._root_node = None
        self._asset_pixel_ratio = -1
        self._base_scale = 1.0
        self._zoom = 1.0
        self._sprite_loader = None
        self._view_pos = vector2.Vector2(0.0, 0.0)
        self._view_aabb = aabb.AABB(0.0, 0.0, 0.0, 0.0)
        self._dirty = True
        self._last_w_size = window.size
        self._sprites = {}  # type: Dict[int, Sprite]
        self._texts = {}  # type: Dict[int, str]
        self._rcopy = render.SDL_RenderCopyEx
        self._rect = rect.SDL_Rect(0, 0, 0, 0)

    def render(self):
        if self._last_w_size != self._window.size:
            self._dirty = True
            self._update_base_scale()
        if not self.root_node.traverse() and not self._dirty:
            return
        if self._dirty:
            self._update_view_aabb()

        w = min(self.window_size)
        x = int(self._view_pos.x * w * self._zoom)
        y = int(self._view_pos.y * w * self._zoom)
        self._renderer.clear()
        image_scale = self._base_scale * self._zoom
        image_scale = image_scale, image_scale
        for nd in self.root_node.query(self._view_aabb):
            if isinstance(nd, node.ImageNode):
                self._render_image(nd, w, image_scale, x, y)
            elif isinstance(nd, node.TextNode):
                self._render_text(nd, w, image_scale, x, y)
        self._dirty = False
        render.SDL_RenderPresent(self.renderer)

    def _render_text(self, nd, w, image_scale, x, y):
        scale_x, scale_y = nd.relative_scale
        n_id = nd.node_id
        if n_id not in self._sprites or n_id not in self._texts \
              or nd.hashkey != self._texts[n_id] \
              or self._sprites[n_id].scale != image_scale:
            self._load_sprite(nd, image_scale)
            self._texts[n_id] = nd.hashkey
        sprite = self._sprites[n_id].sprite
        rel_pos = nd.relative_pos
        self._rect.x = x + int(w * rel_pos.x * self._zoom)
        self._rect.y = y + int(w * rel_pos.y * self._zoom)
        self._rect.w, self._rect.h = sprite.size
        rot_center = nd.rotation_center
        center = rect.SDL_Point(
            int(rot_center.x * w),
            int(rot_center.y * w)
        )
        if self._rcopy(self.renderer, sprite.texture, None, self._rect,
                       nd.relative_angle, center, sprite.flip) == -1:
            raise sdl2.ext.common.SDLError()

    def _render_image(self, nd, w, image_scale, x, y):
        scale_x, scale_y = nd.relative_scale
        n_id = nd.node_id
        if n_id not in self._sprites or n_id not in self._texts \
              or self._sprites[n_id].index != nd.index \
              or self._sprites[n_id].scale != image_scale \
              or self._sprites[n_id].name != nd.image:
            self._load_sprite(nd, image_scale, w)
        sprite = self._sprites[n_id].sprite
        rel_pos = nd.relative_pos
        self._rect.x = x + int(w * rel_pos.x * self._zoom)
        self._rect.y = y + int(w * rel_pos.y * self._zoom)
        self._rect.w, self._rect.h = (
            int(sprite.size[0] * scale_x + 0.5),
            int(sprite.size[1] * scale_y + 0.5)
        )
        rot_center = nd.rotation_center
        center = rect.SDL_Point(
            int(rot_center.x * w),
            int(rot_center.y * w)
        )
        if self._rcopy(self.renderer, sprite.texture, None, self._rect,
                       nd.relative_angle, center, sprite.flip) == -1:
            raise sdl2.ext.common.SDLError()

    def _load_sprite(self, nd, scale, w=None):
        if isinstance(nd, node.ImageNode):
            image_str = nd.image
            if image_str.find(':F:') > -1:  # Pass world unit to float SDF
                image_str += f':w={w}'
            self._sprites[nd.node_id] = Sprite(
                scale,
                self.sprite_loader.load_image(image_str, scale),
                nd.index,
                nd.image
            )
        elif isinstance(nd, node.TextNode):
            size = int(
                nd.font_size * nd.relative_scale[1] * min(self.window_size)
            )
            self._sprites[nd.node_id] = Sprite(
                scale,
                self.sprite_loader.load_text(
                    nd.text,
                    nd.font,
                    size,
                    nd.color,
                    nd.align,
                    nd.spacing,
                    nd.multiline
                ),
                0,
                ''
            )
        x, y = self._sprites[nd.node_id].sprite.size
        sx, sy = nd.relative_scale
        nd.size = (
            round(x * sx, 0) / min(self._window.size),
            round(y * sy, 0) / min(self._window.size)
        )

    def _update_view_aabb(self):
        # type: () -> aabb.AABB
        w, h = self.window_size
        if w < h:
            w, h = 0.5 / self._zoom, 0.5 * (h / w) / self._zoom
        else:
            w, h = 0.5 * (w / h) / self._zoom, 0.5 / self._zoom
        self._view_aabb = aabb.AABB(
            self._view_pos.x + w,
            self._view_pos.y + h,
            w,
            h
        )

    @property
    def root_node(self):
        # type: () -> node.Node
        if self._root_node is None:
            raise RuntimeError('root_node not set.')
        return self._root_node

    @root_node.setter
    def root_node(self, value):
        # type: (node.Node) -> None
        if not isinstance(value, node.Node):
            raise TypeError
        self._root_node = value
        self._dirty = True

    @property
    def asset_pixel_ratio(self):
        # type: () -> int
        if self._asset_pixel_ratio < 0:
            raise RuntimeError('asset_pixel_ratio not set.')
        return self._asset_pixel_ratio

    @asset_pixel_ratio.setter
    def asset_pixel_ratio(self, value):
        # type: (int) -> None
        if not isinstance(value, int):
            raise TypeError
        if value <= 0:
            raise ValueError('Expected non zero, positive int.')
        if value != self._asset_pixel_ratio:
            self._asset_pixel_ratio = value
            self._update_base_scale()
            self._dirty = True

    @property
    def zoom(self):
        # type: () -> float
        return self._zoom

    @zoom.setter
    def zoom(self, value):
        # type: (float) -> None
        if not isinstance(value, float):
            raise TypeError
        if value <= 0.0:
            raise ValueError('Expected non zero, positive float.')
        if value != self._zoom:
            self._zoom = value
            self._dirty = True

    @property
    def view_pos(self):
        # type: () -> vector2.Vector2
        return self._view_pos

    @view_pos.setter
    def view_pos(self, value):
        # type: (vector2.Vector2)
        if not isinstance(value, vector2.Vector2):
            raise TypeError
        if self._view_pos != value:
            self._view_pos = value
            self._dirty = True

    @property
    def window_size(self):
        # type: () -> Tuple[int, int]
        return self._window.size

    @property
    def sprite_loader(self):
        # type: () -> spriteloader.SpriteLoader
        if self._sprite_loader is None:
            raise RuntimeError('sprite_loader not set.')
        return self._sprite_loader

    @sprite_loader.setter
    def sprite_loader(self, value):
        # type: (spriteloader.SpriteLoader) -> None
        if not isinstance(value, spriteloader.SpriteLoader):
            raise TypeError
        self._sprite_loader = value

    def _update_base_scale(self):
        ws = self._window.size
        self._base_scale = min(ws) / self._asset_pixel_ratio
        self._last_w_size = ws
