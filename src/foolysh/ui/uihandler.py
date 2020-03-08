"""Provides the UIHandler class."""

from dataclasses import dataclass
from enum import Enum
from typing import Any, Callable, Dict, Optional, Tuple, Union
import sdl2

from .uinode import UINode
from .. import eventhandler
from ..tools import clock
from ..tools import vec2
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
    UP = 2  # NOT IMPLEMENTED!
    ENTER_FOCUS = 3
    EXIT_FOCUS = 4
    RETURN = 5
    INPUT = 6
    RESET = 7
    BACKSPACE = 8
    BLINK = 9


@dataclass
class HandlerState:
    """The various state variables in UIHandler."""
    text_input_active: bool = False
    current_focus: Optional[int] = None
    current_down: Optional[int] = None
    need_render: bool = False



CallbackDict = Dict[Tuple[int, EventType],
                    Tuple[Callable[..., None], Tuple[Any, ...], Dict[str, Any]]]
MousePos = Tuple[vec2.Vec2, Union[vec2.Vec2, None], Union[vec2.Vec2, None]]

class UIHandler:
    """
    Callable handler class for UI update and event handling.

    Args:
        uiroot: :class:`~foolysh.ui.uinode.UINode` -> the root node of the UI.
        event_handler: :class:`~foolysh.eventhandler.EventHandler` -> app
            eventhandler to get potential text input, if needed.
    """

    def __init__(self, uiroot: UINode,
                 event_handler: eventhandler.EventHandler) -> None:
        self._uiroot = uiroot
        self._event_handler = event_handler
        self._nodes: Dict[int, node.Node] = {}
        self._callbacks: CallbackDict = {}
        self._state: HandlerState = HandlerState()
        self._clock: clock.Clock = clock.Clock()

    def add_node(self, nd: node.Node) -> None:
        """Add a :class:`~foolysh.ui.uinode.UINode` to be handled."""
        if not isinstance(nd, UINode):
            raise TypeError('Can only register type: UINode.')
        self._nodes[nd.node_id] = nd

    def register_event(self, nd: node.Node, e_type: EventType,
                       event_cb: Callable, *args: Tuple,
                       **kwargs: Dict) -> None:
        """
        Register an event callback for a :class:`~foolysh.ui.uinode.UINode`.
        Only one callback per :class:`EventType` and
        :class:`~foolysh.ui.uinode.UINode` can be registered,

        .. info::
            For text input events, the keyword argument `text` is injected into
            the kwargs dict, before calling the callback, containing the text
            received in the event.

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

    def __call__(self, mouse_pos: vec2.Vec2,
                 mouse_down: Optional[vec2.Vec2] = None,
                 mouse_up: Optional[vec2.Vec2] = None,
                 enter: bool = False, backspace: bool = False) -> bool:
        """
        Method to be called each frame to evaluate and execute appropriate
        callbacks.

        Args:
            mouse_pos: :class:`~foolysh.tools.vec2.Vec2` -> current mouse
                position.
            mouse_down: :class:`~foolysh.tools.vec2.Vec2` -> if set indicates
                where the mouse (or finger) down started.
            mouse_pos: :class:`~foolysh.tools.vec2.Vec2` -> if set indicates
                where the mouse (or finger) down ended.
            enter: ``bool`` -> whether the Enter key was pressed.
            backspace: ``bool`` -> whether the BackSpace key was pressed.

        Returns:
            Need more frames rendered flag, that can be set by UINodes.
        """
        down = mouse_down is not None and mouse_up is None
        click = mouse_down is not None and mouse_up is not None
        last_input = self._event_handler.last_text_input
        self._blink_events()

        if not (enter or backspace or down or click) and last_input is None:
            if self._state.need_render:
                self._state.need_render = False
                return True
            return False

        new_focus, same_focus = self.\
            _exc_mouse_enter_cb(down, click, (mouse_pos, mouse_down, mouse_up),
                                enter)
        curfoc = self._state.current_focus
        ret_focus = new_focus or same_focus
        if click and not ret_focus and curfoc is not None:  # EXIT_FOCUS
            self._nodes[curfoc].focus = False
            k = curfoc, EventType.EXIT_FOCUS
            self._try_cb_exec(k)
            self._state.current_focus = None
            sdl2.SDL_StopTextInput()
        elif click and new_focus:                           # ENTER_FOCUS
            k = curfoc, EventType.ENTER_FOCUS
            self._try_cb_exec(k)

        if last_input is not None and curfoc is not None:
            k = curfoc, EventType.INPUT
            self._try_cb_exec(k, text=last_input)
        elif backspace and curfoc is not None:
            k = curfoc, EventType.BACKSPACE
            self._try_cb_exec(k)

        if self._state.need_render:
            self._state.need_render = False
            return True
        return False

    def _try_cb_exec(self, k, **add_kwargs):
        if k in self._callbacks:
            callback, args, kwargs = self._callbacks[k]
            kwargs.update(add_kwargs)
            callback(*args, **kwargs)

    def _exc_mouse_enter_cb(self, down: bool, click: bool, mouse_pos: MousePos,
                            enter: bool):
        new_focus = same_focus = False
        sort_callbacks = sorted(self._callbacks,
                                key=lambda x: self._nodes[x[0]].relative_depth,
                                reverse=True)
        for node_id, event_t in sort_callbacks:
            nd_bb = self._nodes[node_id].aabb
            exc = False
            if click and event_t in (EventType.CLICK, EventType.INPUT):
                exc, click, new_focus, same_focus = self.\
                    _handle_click(node_id, nd_bb, mouse_pos, click, new_focus,
                                  event_t)
            elif down and event_t == EventType.DOWN:
                exc, down = self._handle_down(node_id, nd_bb, mouse_pos, down)
            elif node_id == self._state.current_down and event_t == EventType.RESET:
                exc = self._handle_reset(nd_bb, mouse_pos)
            if enter and event_t == EventType.RETURN:
                if self._nodes[node_id].focus:
                    exc = True

            if exc:
                callback, args, kwargs = self._callbacks[(node_id, event_t)]
                callback(*args, **kwargs)
        return new_focus, same_focus

    def _handle_click(self, node_id, nd_bb, mouse_pos, click, new_focus,
                      event_t) -> Tuple[bool, bool, bool, bool]:
        # pylint: disable=too-many-arguments
        valid = nd_bb.inside_tup(*mouse_pos[2]) \
                and nd_bb.inside_tup(*mouse_pos[1])
        exc = False
        same_focus = False
        self._state.current_down = None
        if valid:
            exc = event_t == EventType.CLICK
            if self._state.current_focus != node_id:
                self._nodes[node_id].focus = True
                if self._state.current_focus is not None:
                    self._nodes[self._state.current_focus].focus = False
                    k = self._state.current_focus, EventType.EXIT_FOCUS
                    self._try_cb_exec(k)
                self._state.current_focus = node_id
                if (node_id, EventType.INPUT) in self._callbacks:
                    sdl2.SDL_StartTextInput()
                new_focus = True
                click = False  # Prevents further click callbacks
            else:
                same_focus = True
        return exc, click, new_focus, same_focus

    def _handle_down(self, node_id, nd_bb, mouse_pos, down):
        valid = nd_bb.inside_tup(*mouse_pos[0]) \
                and nd_bb.inside_tup(*mouse_pos[1])
        exc = False
        if valid:
            down = False  # Prevents further down callbacks
            if node_id != self._state.current_down:
                exc = True
                self._state.current_down = node_id
        return exc, down

    def _handle_reset(self, nd_bb, mouse_pos):
        valid = nd_bb.inside_tup(*mouse_pos[0]) \
                and nd_bb.inside_tup(*mouse_pos[1])
        exc = False
        if not valid:
            exc = True
            self._state.current_down = None
        return exc

    def _blink_events(self) -> None:
        self._clock.tick()
        for node_id, event_t in self._callbacks:
            if event_t == EventType.BLINK:
                func, args, kwargs = self._callbacks[(node_id, event_t)]
                kwargs['dt'] = self._clock.get_dt()
                kwargs['frame_time'] = self._clock.get_time()
                func(*args, **kwargs)

    @property
    def need_render(self) -> bool:
        """
        Flag that can be set by :class:`~foolysh.ui.uinode.UINode` classes.
        Forces the entire scene to be rendered next frame.

        :setter:
            Set to ``True`` to ensure rendering of the next frame.
        """
        return self._state.need_render

    @need_render.setter
    def need_render(self, value: bool) -> None:
        if not isinstance(value, bool):
            raise TypeError
        self._state.need_render = value
