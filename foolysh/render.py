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


Sprite = namedtuple('Sprite', ['image', 'scale', 'sprite'])


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
        self._sprites = {}  # type: Dict[int, Sprite]

    def render(self):
        if not self.root_node.traverse() and not self._dirty:
            return
        if self._dirty:
            self._update_view_aabb()

        w = min(self.window_size)
        xo = int(self._view_pos.x * w * self._zoom)
        yo = int(self._view_pos.y * w * self._zoom)
        r = rect.SDL_Rect(0, 0, 0, 0)
        rcopy = render.SDL_RenderCopyEx
        renderer = self.sdlrenderer
        self._renderer.clear()
        for nd in self.root_node.query(self._view_aabb):
            if not isinstance(nd, node.ImageNode):
                continue
            nd_scale_x, nd_scale_y = nd.relative_scale
            scale_x = nd_scale_x * self._base_scale * self._zoom
            scale_y = nd_scale_y * self._base_scale * self._zoom
            if nd.node_id not in self._sprites or \
                 self._sprites[nd.node_id].scale != (scale_x, scale_y):
                self._load_sprite(nd, (scale_x, scale_y))
            sprite = self._sprites[nd.node_id].sprite
            rel_pos = nd.relative_pos
            r.x = xo + int(w * rel_pos.x * self._zoom)
            r.y = yo + int(w * rel_pos.y * self._zoom)
            r.w, r.h = sprite.size
            rot_center = nd.rotation_center
            center = rect.SDL_Point(
                int(rot_center.x * w), 
                int(rot_center.y * w)
            )
            if rcopy(renderer, sprite.texture, None, r, nd.relative_angle,
                     center, sprite.flip) == -1:
                raise SDLError()
        self._dirty = False
        render.SDL_RenderPresent(renderer)

    def _load_sprite(self, nd, scale):
        self._sprites[nd.node_id] = Sprite(
            nd.image, 
            scale,
            self.sprite_loader.load_image(nd.image, scale) 
        )
        x, y = self._sprites[nd.node_id].sprite.size
        nd.size = (
            x / self._base_scale * self._asset_pixel_ratio,
            y / self._base_scale * self._asset_pixel_ratio
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
            self._base_scale = min(self._window.size) / value 
            self._asset_pixel_ratio = value
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
