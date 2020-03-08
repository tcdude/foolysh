"""
Provides the Label class for building UI components.
"""

from typing import Optional

from . import frame
from ..tools.common import COLOR

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


class Label(frame.Frame):
    """
    A label to display text on a frame.

    See :class:`~foolysh.ui.frame.Frame` for keyword arguments and properties
    that are inherited. For text specific keyword arguments see
    :class:`~foolysh.scene.node.TextNode`.

    Args:
        margin: ``Optional[float]`` -> margin when align is either 'left' or
            'right'
        **kwargs: Keyword arguments for :class:`~foolysh.ui.frame.Frame` and
            :class:`~foolysh.scene.node.TextNode` initialization.
    """

    def __init__(self, margin: Optional[float] = 0.0, **kwargs) -> None:
        super().__init__(**kwargs)
        kwargs['pos'] = 0, 0
        self._txt_node = self.attach_text_node(**kwargs)
        self._margin = margin

    def _place_txt_node(self, txt_node: "foolysh.scene.node.TextNode") -> None:
        if txt_node.size != (0, 0):
            if txt_node.align == 'left':
                x = self.border_thickness + self.corner_radius + self._margin
            elif txt_node.align == 'center':
                x = (self.size[0] - txt_node.size[0]) / 2
            else:
                x = self.size[0] - self.border_thickness - self.corner_radius
                x -= txt_node.size[0] + self._margin
            y = (self.size[1] - txt_node.size[1]) / 2
        else:
            x = y = 0
            self.ui_handler.need_render = True
        txt_node.pos = x, y

    def _update(self):
        self._place_txt_node(self._txt_node)
        super()._update()

    @property
    def text(self) -> str:
        """The current text."""
        return self._txt_node.text

    @text.setter
    def text(self, value: str) -> None:
        self._txt_node.text = value
        self.dirty = True

    @property
    def font(self) -> str:
        """The current asset path to the font."""
        return self._txt_node.font

    @font.setter
    def font(self, value: str) -> None:
        self._txt_node.font = value
        self.dirty = True

    @property
    def font_size(self) -> float:
        """The font size."""
        return self._txt_node.font_size

    @font_size.setter
    def font_size(self, value: float) -> None:
        self._txt_node.font_size = value
        self.dirty = True

    @property
    def text_color(self) -> COLOR:
        """
        The text color.

        :setter:
            * ``int`` -> sets all components of the color to the specified
                value.
            * 3-/4-``tuple`` of ``int`` -> RGB or RGBA color. The alpha value
                defaults to 255, if RGB only is provided.
        """
        return self._txt_node.text_color

    @text_color.setter
    def text_color(self, value: COLOR) -> None:
        self._txt_node.text_color = value
        self.dirty = True

    @property
    def align(self) -> str:
        """Alignment of the text 'left', 'center' or 'right'."""
        return self._txt_node.align

    @align.setter
    def align(self, value: str) -> None:
        self._txt_node.align = value
        self.dirty = True
