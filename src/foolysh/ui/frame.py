"""
Provides the Frame class for building UI components.
"""

from dataclasses import dataclass
from typing import Optional
from typing import Tuple
from typing import Union

from PIL import Image

from ..ui import ObjectState
from ..ui import uinode
from ..scene.node import ImageNode
from ..tools import sdf
from ..tools import vector2
from ..tools.common import COLOR
from ..tools.common import valid_color

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


@dataclass
class _Colors:
    """Hold color combinations for Frame."""
    frame_normal: Optional[COLOR] = None
    border_normal: Optional[COLOR] = None
    frame_hover: Optional[COLOR] = None
    border_hover: Optional[COLOR] = None
    frame_down: Optional[COLOR] = None
    border_down: Optional[COLOR] = None
    frame_disabled: Optional[COLOR] = None
    border_disabled: Optional[COLOR] = None


class Frame(uinode.UINode):
    """
    Representation of a frame.

    Args:
        size: ``Tuple[float, float]`` -> width and height of the frame.
        pos: ``Union[Tuple[float, float],``
            :class:`~foolysh.tools.vector2.Vector2`
            ``]`` -> position of the frame.
        frame_color: ``Optional[COLOR]`` -> color of the frame.
        border_thickness: ``Optional[float]`` -> thickness of the border.
        border_color: ``Optional[COLOR]`` -> color of the border.
        corner_radius: ``Optional[COLOR]`` -> rounded corner radius.
        frame_color_hover: ``Optional[COLOR]`` -> frame color on hover.
        border_color_hover: ``Optional[COLOR]`` -> border color on hover.
        frame_color_down: ``Optional[COLOR]`` -> frame color on mouse/finger
            down.
        border_color_down: ``Optional[COLOR]`` -> border color on mouse/finger
            down.
        frame_color_disabled: ``Optional[COLOR]`` -> frame color when disabled.
        border_color_disabled: ``Optional[COLOR]`` -> border color when
            disabled.
    """

    def __init__(self, size: Tuple[float, float],
                 pos: Union[Tuple[float, float], vector2.Vector2],
                 name: Optional[str] = 'Unnamed Frame',
                 frame_color: Optional[COLOR] = (160, 160, 160, 80),
                 border_thickness: Optional[float] = 0.0,
                 border_color: Optional[COLOR] = (255, 255, 255, 255),
                 corner_radius: Optional[float] = 0.0,
                 frame_color_hover: Optional[COLOR] = None,
                 border_color_hover: Optional[COLOR] = None,
                 frame_color_down: Optional[COLOR] = None,
                 border_color_down: Optional[COLOR] = None,
                 frame_color_disabled: Optional[COLOR] = None,
                 border_color_disabled: Optional[COLOR] = None) -> None:
        super().__init__(name)
        self.size = size
        self.pos = pos
        self._colors = _Colors()
        self.frame_color = frame_color
        self._border_thickness = 0.0
        self.border_thickness = border_thickness
        self.border_color = border_color
        self._corner_radius = 0.0
        self.corner_radius = corner_radius
        self.hover_colors = frame_color_hover, border_color_hover
        self.down_colors = frame_color_down, border_color_down
        self.disabled_colors = frame_color_disabled, border_color_disabled
        self._window_size = 0, 0
        self._multi_sampling = -1
        self._image_node = self.attach_image_node(f'{self.name}: ImageNode')

    def generate(self, window_size, multi_sampling=1):
        sx, sy = self.size
        width, height = sx * window_size[0], sy * window_size[1]
        corner_radius = self._corner_radius * min(window_size)
        border_thickness = self._border_thickness * min(window_size)

    def get_image(
            self,
            window_size: Tuple[int, int],
            multi_sampling: Optional[int] = 1,
            state: Optional[ObjectState] = ObjectState.NORMAL
        ) -> Image:
        """
        Retrieve the image for the specified window size and state.

        Args:
            window_size: ``Tuple[int, int]`` -> screen resolution.
            multi_sampling: ``Optional[int]`` -> multiplier of the size, the
                SDF is computed.
            state: ``Optional[ObjectState]`` -> state of the frame.
        """
        frame_color = self._colors.frame_normal[:3]
        border_color = self._colors.border_normal[:3]
        if state == ObjectState.HOVER:
            frame_color = self._colors.frame_hover[:3] or frame_color
            border_color = self._colors.border_hover[:3] or border_color
        elif state == ObjectState.DOWN:
            frame_color = self._colors.frame_down[:3] or frame_color
            border_color = self._colors.border_down[:3] or border_color
        elif state == ObjectState.DISABLED:
            frame_color = self._colors.frame_disabled[:3] or frame_color
            border_color = self._colors.border_disabled[:3] or border_color
        width, height = (
            int(resolution[0] * self._width + 0.5),
            int(resolution[1] * self._height + 0.5),
        )
        corner_radius = int(round(min(resolution) * self._corner_radius, 0))
        border_thickness = int(
            round(min(resolution) * self._border_thickness + 0.5, 0)
        )
        return sdf.framed_box_im(
            width,
            height,
            corner_radius,
            border_thickness,
            frame_color,
            border_color
        )

    # Properties

    @property
    def frame_color(self) -> COLOR:
        """
        ``Tuple[int, int, int[, int]]`` -> color of the frame.

        :setter:
            ``Tuple[int, int, int[, int]]``
        """
        return self._colors.frame_normal

    @frame_color.setter
    def frame_color(self, color: COLOR) -> None:
        self._colors.frame_normal = valid_color(color)

    @property
    def border_thickness(self) -> float:
        """
        ``float`` -> thickness of the border.

        :setter:
            ``float``
        """
        return self._border_thickness

    @border_thickness.setter
    def border_thickness(self, thickness: float) -> None:
        self._border_thickness = thickness

    @property
    def border_color(self) -> COLOR:
        """
        ``Tuple[int, int, int[, int]]`` -> color of the border.

        :setter:
            ``Tuple[int, int, int[, int]]``
        """
        return self._colors.border_normal

    @border_color.setter
    def border_color(self, color: COLOR) -> None:
        self._colors.border_normal = valid_color(color)

    @property
    def corner_radius(self) -> float:
        """
        ``float`` -> corner radius.

        :setter:
            ``float``
        """
        return self._border_color

    @corner_radius.setter
    def corner_radius(self, radius: float) -> None:
        self._corner_radius = radius

    @property
    def hover_colors(self) -> COLOR:
        """
        ``Tuple[COLOR, COLOR]`` -> color of the frame and border on hover.

        :setter:
            ``Tuple[COLOR, COLOR]``
        """
        return self._colors.frame_hover, self._colors.border_hover

    @hover_colors.setter
    def hover_colors(self, colors: Tuple[COLOR, COLOR]) -> None:
        if not isinstance(colors, tuple):
            raise TypeError('Expected tuple.')
        if len(colors) != 2:
            raise ValueError('Expected 2 value tuple.')
        self._colors.frame_hover = valid_color(colors[0])
        self._colors.border_hover = valid_color(colors[1])

    @property
    def down_colors(self) -> COLOR:
        """
        ``Tuple[COLOR, COLOR]`` -> color of the frame and border on mouse/finger
            down.

        :setter:
            ``Tuple[COLOR, COLOR]``
        """
        return self._colors.frame_down, self._colors.border_down

    @down_colors.setter
    def down_colors(self, colors: Tuple[COLOR, COLOR]) -> None:
        if not isinstance(colors, tuple):
            raise TypeError('Expected tuple.')
        if len(colors) != 2:
            raise ValueError('Expected 2 value tuple.')
        self._colors.frame_down = valid_color(colors[0])
        self._colors.border_down = valid_color(colors[1])

    @property
    def disabled_colors(self) -> COLOR:
        """
        ``Tuple[COLOR, COLOR]`` -> color of the frame and border when disabled.

        :setter:
            ``Tuple[COLOR, COLOR]``
        """
        return self._colors.frame_disabled, self._colors.border_disabled

    @disabled_colors.setter
    def disabled_colors(self, colors: Tuple[COLOR, COLOR]) -> None:
        if not isinstance(colors, tuple):
            raise TypeError('Expected tuple.')
        if len(colors) != 2:
            raise ValueError('Expected 2 value tuple.')
        self._colors.frame_disabled = valid_color(colors[0])
        self._colors.border_disabled = valid_color(colors[1])
