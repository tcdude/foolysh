"""
Provides the Frame class for building UI components.
"""

from dataclasses import dataclass
from typing import List
from typing import Optional
from typing import Tuple
from typing import Union

from PIL import Image

from ..ui import ObjectState
from ..ui import uinode
from ..scene.node import ImageNode
from ..tools import sdf
from ..tools import vec2
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
class FrameSDF:
    """
    Parameters to build the SDF (except size, which is stored in the Node).
    """
    frame_color: COLOR
    border_color: COLOR
    corner_radius: float
    border_thickness: float
    multi_sampling: int
    alpha: int


class Frame(uinode.UINode):
    """
    A frame to place content on. Currently, overlapping content will not be
    cropped.

    Args:
        size: ``Tuple[float, float]`` -> width and height of the frame.
        pos: ``Union[Tuple[float, float],``
            :class:`~foolysh.tools.vec2.Vec2`
            ``]`` -> position of the frame.
        frame_color: ``Optional[COLOR]`` -> color of the frame.
        border_thickness: ``Optional[float]`` -> thickness of the border.
        border_color: ``Optional[COLOR]`` -> color of the border.
        corner_radius: ``Optional[COLOR]`` -> rounded corner radius.
        multi_sampling: ``Optional[int]`` -> multi sampling applied to the SDF.
        alpha: ``Optional[int]`` -> alpha/transparency value.
        **unused_kwargs: keyword arguments, that are ignored.
    """

    def __init__(self, name: Optional[str] = 'Unnamed Frame',
                 size: Optional[Tuple[float, float]] = (0.0, 0.0),
                 pos: Union[Tuple[float, float], vec2.Vec2] = vec2.Vec2(),
                 frame_color: Optional[COLOR] = (160, 160, 160),
                 border_thickness: Optional[float] = 0.0,
                 border_color: Optional[COLOR] = (255, 255, 255),
                 corner_radius: Optional[float] = 0.0,
                 multi_sampling: Optional[int] = 1,
                 alpha: Optional[int] = 255, **unused_kwargs) -> None:
        # pylint: disable=too-many-arguments

        if size == (0.0, 0.0):
            raise ValueError('Expected size different to 0, 0.')
        super().__init__(name, size, pos)
        self._frame_sdf: List[FrameSDF] = []
        self._im_node = self.attach_image_node(name + '_Image')
        self._im_node.depth = 0
        self.add_state(frame_color, border_thickness, border_color,
                       corner_radius, multi_sampling, alpha)

    def add_state(self, frame_color: Optional[COLOR] = (160, 160, 160),
                  border_thickness: Optional[float] = 0.0,
                  border_color: Optional[COLOR] = (255, 255, 255),
                  corner_radius: Optional[float] = 0.0,
                  multi_sampling: Optional[int] = 1,
                  alpha: Optional[int] = 255) -> int:
        """
        Add an additional state to the Frame.

        Args:
            frame_color: ``Optional[COLOR]`` -> color of the frame.
            border_thickness: ``Optional[float]`` -> thickness of the border.
            border_color: ``Optional[COLOR]`` -> color of the border.
            corner_radius: ``Optional[COLOR]`` -> rounded corner radius.
            multi_sampling: ``Optional[int]`` -> multi sampling applied to the
                SDF.
            alpha: ``Optional[int]`` -> alpha/transparency value.

        Returns:
            ``int`` -> the index of the newly added state.
        """
        # pylint: disable=too-many-arguments
        self._frame_sdf.append(FrameSDF(frame_color, border_color,
                                        corner_radius, border_thickness,
                                        multi_sampling, alpha))
        return self._im_node.add_image(self._sdfstr(len(self._frame_sdf) - 1))

    def _sdfstr(self, index: int):
        frame_sdf = self._frame_sdf[index]
        return sdf.framed_box_str(*self.size, frame_sdf.corner_radius,
                                  frame_sdf.border_thickness,
                                  frame_sdf.frame_color[:3],
                                  frame_sdf.border_color[:3],
                                  frame_sdf.multi_sampling,
                                  frame_sdf.alpha)

    def _update(self) -> None:
        self._im_node.clear_images()
        for i in range(len(self._frame_sdf)):
            self._im_node.add_image(self._sdfstr(i))
        super()._update()

    # Properties

    @property
    def index(self) -> int:
        """
        ``int`` -> index of the active state.

        :setter:
            ``int`` -> a valid state index to change to.
        """
        return self._im_node.index

    @index.setter
    def index(self, value: int) -> None:
        self._im_node.index = value
        self.dirty = True

    @property
    def frame_color(self) -> COLOR:
        """
        ``Tuple[int, int, int[, int]]`` -> color of the active state of the
        frame.

        :setter:
            ``Tuple[int, int, int[, int]]`` -> sets the color for the active
            state.
        """
        return self._frame_sdf[self._im_node.index].frame_color

    @frame_color.setter
    def frame_color(self, color: COLOR) -> None:
        self._frame_sdf[self._im_node.index].frame_color = valid_color(color)
        self.dirty = True

    @property
    def border_thickness(self) -> float:
        """
        ``float`` -> thickness of the active state of the border.

        :setter:
            ``float``
        """
        return self._frame_sdf[self._im_node.index].border_thickness

    @border_thickness.setter
    def border_thickness(self, thickness: float) -> None:
        self._frame_sdf[self._im_node.index].border_thickness = thickness
        self.dirty = True

    @property
    def border_color(self) -> COLOR:
        """
        ``Tuple[int, int, int[, int]]`` -> color of the active state of the
        border.

        :setter:
            ``Tuple[int, int, int[, int]]``
        """
        return self._frame_sdf[self._im_node.index].border_color

    @border_color.setter
    def border_color(self, color: COLOR) -> None:
        self._frame_sdf[self._im_node.index].border_color = valid_color(color)
        self.dirty = True

    @property
    def corner_radius(self) -> float:
        """
        ``float`` -> corner radius of the active state.

        :setter:
            ``float``
        """
        return self._frame_sdf[self._im_node.index].corner_radius

    @corner_radius.setter
    def corner_radius(self, radius: float) -> None:
        self._frame_sdf[self._im_node.index].corner_radius = radius
        self.dirty = True

    @property
    def multi_sampling(self) -> int:
        """
        ``int`` -> multi sampling of the active state.

        :setter:
            ``int``
        """
        return self._frame_sdf[self._im_node.index].multi_sampling

    @multi_sampling.setter
    def multi_sampling(self, value: int) -> None:
        self._frame_sdf[self._im_node.index].multi_sampling = value
        self.dirty = True

    @property
    def alpha(self) -> int:
        """
        ``int`` -> alpha/transparency of the active state.

        :setter:
            ``int``
        """
        return self._frame_sdf[self._im_node.index].alpha

    @alpha.setter
    def alpha(self, value: int) -> None:
        self._frame_sdf[self._im_node.index].alpha = value
        self.dirty = True
