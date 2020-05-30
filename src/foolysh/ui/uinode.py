"""
Provides a basic UI Layer to design simple graphical user interfaces or HUDs.
Every element is based on :class:`~foolysh.ui.UIObject` which can be
hierarchically placed on multiple base nodes, each representing a different
coordinate system (e.g. :attr:`~foolysh.app.App.ui.top_left`,
:attr:`~foolysh.app.App.ui.center`, :attr:`~foolysh.app.App.ui.bottom_left`, ...
).

:UI Elements:
    The following elements are provided.

    * :class:`~foolysh.ui.frame.Frame` provides a frame.
    * :class:`~foolysh.ui.label.Label` provides a label.
    * :class:`~foolysh.ui.button.Button` provides a button.
    * :class:`~foolysh.ui.button.RadioButton` provides a radio button.
    * :class:`~foolysh.ui.button.ToggleButton` provides a toggle button.
    * :class:`~foolysh.ui.input.Input` provides an input field.

"""

import abc
from typing import Tuple

from ..scene import node
from ..tools import vec2

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


UIHANDLER_INSTANCE = None


class UINode(node.Node):
    """
    Node subclass containing shared properties of UI elements. This can only be
    instantiated after :class:`~foolysh.app.App` has been initialized.

    Args:
        name:
        size:
        pos:
    """
    def __init__(self, name: str = 'Unnamed UINode',
                 size: Tuple[float, float] = (0.0, 0.0),
                 pos: vec2.Vec2 = vec2.Vec2(), **kwargs) -> None:
        if UIHANDLER_INSTANCE is None:
            raise RuntimeError('UINode objects can only be instantiated after '
                               'UIHANDLER_INSTANCE has been set (i.e. after '
                               'the __init__ method of foolysh.app.App has '
                               'been called).')
        super().__init__(name, **kwargs)
        self.size = size
        self.pos = pos
        self._has_focus = False
        self.__dirty = True
        self.__first = 2

    def update(self):
        """Performs a complete update of this the :class:`UINode`."""
        if self.__dirty or self.__first:
            self._update()
        self.__first = max(0, self.__first - 1)
        if not self.__first:
            self.__dirty = False

    @abc.abstractmethod
    def _update(self) -> None:
        pass

    @property
    def dirty(self) -> bool:
        """Whether the Node is dirty."""
        return self.__dirty

    @dirty.setter
    def dirty(self, value: bool) -> None:
        if not isinstance(value, bool):
            raise TypeError('Expected type bool.')
        self.propagate_dirty()
        self.__dirty = value

    @property
    def focus(self) -> bool:
        """Whether the Node is focused."""
        return self._has_focus

    @focus.setter
    def focus(self, value: bool) -> None:
        if not isinstance(value, bool):
            raise TypeError('Expected bool.')
        self._has_focus = value

    @property
    def ui_handler(self) -> "foolysh.ui.uihandler.UIHandler":
        """
        Provides access to the :class:`~foolysh.ui.uihandler.UIHandler` instance
        used in the running :class:`~foolysh.app.App`.
        """
        return UIHANDLER_INSTANCE
