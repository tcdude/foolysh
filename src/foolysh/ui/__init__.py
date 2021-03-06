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

import enum

from . import uihandler
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




class UIState(enum.Enum):
    """Possible states a UI element can be in."""
    NORMAL = 0
    ACTIVE = 1
    DISABLED = 2
