"""Provides the UIHandler class."""

from enum import Enum
from typing import Any
from typing import Callable
from typing import Dict
from typing import Optional
from typing import Tuple

from . import uinode
from ..tools import vector2
from ..scene import node

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


class EventType(Enum):
    """
    Event types.
    """
    CLICK = 0
    DOWN = 1
    UP = 2
    ENTER_FOCUS = 3
    EXIT_FOCUS = 4
    RETURN = 5
    INPUT = 6


CallbackDict = Dict[Tuple[int, EventType],
                    Tuple[Callable[..., None], Tuple[Any, ...], Dict[str, Any]]]


class UIHandler:
    """
    Callable handler class for UI update and event handling.

    Args:
        uiroot: :class:`~foolysh.ui.uinode.UINode` -> the
    """

    def __init__(self, uiroot: uinode.UINode) -> None:
        self._uiroot = uiroot
        self._nodes: Dict[int, node.Node] = {}
        self._callbacks: CallbackDict = {}

    def add_node(self, nd: node.Node) -> None:
        """Add a :class:`~foolysh.ui.uinode.UINode` to be handled."""
        if not isinstance(nd, uinode.UINode):
            raise TypeError('Can only register type: UINode.')
        self._nodes[nd.node_id] = nd

    def register_event(self, nd: node.Node, e_type: EventType,
                       event_cb: callable, *args: Tuple,
                       **kwargs: Dict) -> None:
        """
        Register an event callback for a :class:`~foolysh.ui.uinode.UINode`.
        Only one callback per :class:`EventType` and
        :class:`~foolysh.ui.uinode.UINode` can be registered,

        .. warn::
            Subsequent calls with the same combination, overwrite the current
            callback.

        Args:
            nd: :class:`~foolysh.scene.node.Node` -> The UINode to register the
                event to.
            e_type: :class:`EventType`.
            event_cb: ``callable`` -> callback to register. 
            *args: Positional arguments for the callback.
            **kwargs: Keyword arguments for the callback.
        """
        if nd.node_id not in self._nodes:
            raise ValueError(f'Provided Node "{repr(nd)}" has not been added '
                             f'yet to be handled.')
        self._callbacks[(nd.node_id, e_type)] = event_cb, args, kwargs

    def __call__(self, mouse_pos: vector2.Vector2,
                 mouse_down: Optional[vector2.Vector2] = None,
                 mouse_up: Optional[vector2.Vector2] = None,
                 enter: bool = False) -> None:
        """
        Method to be called each frame to evaluate and execute appropriate
        callbacks.

        Args:
            mouse_pos: :class:`~foolysh.tools.vector2.Vector2` -> current mouse
                position.
            mouse_down: :class:`~foolysh.tools.vector2.Vector2` -> if set
                indicates where the mouse (or finger) down started.
            mouse_pos: :class:`~foolysh.tools.vector2.Vector2` -> if set
                indicates where the mouse (or finger) down ended.
            enter: ``bool`` -> whether the Enter key was pressed.
        """
        if mouse_down is None and mouse_up is None:
            pass
        elif mouse_down is not None and mouse_up is None:
            pass
        elif mouse_down is not None and mouse_up is not None:
            pass
        else:
            raise RuntimeError('Unhandled combination of optional arguments.')
