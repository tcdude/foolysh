"""
Provides drag and drop logic to :class:`~foolysh.scene.node.Node` instances.
"""

from dataclasses import dataclass
from typing import Callable
from typing import Dict
from typing import Optional
from typing import Tuple
from uuid import uuid4

from foolysh.tools import vec2
import sdl2

from .tools import aabb
from . import app
from .scene import node

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


MAX_SPEED = 4.0  # To identify illegal drags happening on android


@dataclass
class DragEntry:
    """
    Class for storing typed information of drag and drop enabled nodes.
    """
    node: node.Node
    drag_callback: Callable
    drag_args: Tuple
    drop_callback: Callable
    drop_args: Tuple


@dataclass
class DragInfo:
    """Information stored about a drag operation."""
    start_pos: vec2.Vec2 = None
    last_mouse: vec2.Vec2 = None
    active: int = -1


class DragDrop:
    """
    A system that handles drag and drop interaction with
    :class:`~foolysh.scene.node.Node` types.

    Args:
        app_instance: :class:`~foolysh.app.App` -> the App instance running.
        drag_threshold: ``float`` -> threshold in world units, until it counts
            as drag and drop action.
        watch_button: ``int`` -> optional which button to watch.
        (default=:const:`sdl2.SDL_BUTTON_LEFT`)
    """

    def __init__(
            self,
            app_instance: app.App,
            drag_threshold: Optional[float] = 0.025,
            watch_button: Optional[int] = sdl2.SDL_BUTTON_LEFT
    ) -> None:
        self._app = app_instance
        self._drag_threshold = drag_threshold
        self._drag_nodes: Dict[int, DragEntry] = {}
        self._info: DragInfo = DragInfo()
        self._ids = uuid4().hex, uuid4().hex, uuid4().hex
        if self._app.isandroid:
            self._watch_button = None
            self._app.event_handler.listen(
                self._ids[0],
                sdl2.SDL_FINGERDOWN,
                self._mouse_down
            )
            self._app.event_handler.listen(
                self._ids[1],
                sdl2.SDL_FINGERUP,
                self._mouse_up
            )
        else:
            self._watch_button = watch_button
            self._app.event_handler.listen(
                self._ids[0],
                sdl2.SDL_MOUSEBUTTONDOWN,
                self._mouse_down
            )
            self._app.event_handler.listen(
                self._ids[1],
                sdl2.SDL_MOUSEBUTTONUP,
                self._mouse_up
            )
        self._app.task_manager.add_task(self._ids[2], self._process, 0, True)
        self._app.task_manager[self._ids[2]].pause()

    def enable(self, drag_node: node.Node,
               drag_callback: Optional[Callable] = None,
               drag_args: Optional[Tuple] = (),
               drop_callback: Optional[Callable] = None,
               drop_args: Optional[Tuple] = ()) -> None:
        # pylint: disable=too-many-arguments
        """
        Enable drag and drop for a :class:`foolysh.scene.node.Node`.

        Args:
            drag_node: :class:`foolysh.scene.node.Node`
            drag_callback: ``Optional[Callable]`` to call on drag start. If the
                callback returns ``False``, no drag manipulation will be
                executed.
            drag_args: ``Optional[Tuple]`` positional arguments for the
                drag_callback callable.
            drop_callback: ``Optional[Callable]`` to call on drop.
            drop_args: ``Optional[Tuple]`` positional arguments for the
                drop_callback callable.
        """
        if not self._drag_nodes:
            self._app.task_manager[self._ids[2]].resume()
        self._drag_nodes[drag_node.node_id] = DragEntry(
            node=drag_node,
            drag_callback=drag_callback,
            drag_args=drag_args,
            drop_callback=drop_callback,
            drop_args=drop_args
        )

    def disable(self, drag_node: node.Node) -> None:
        """
        Disable drag and drop for a :class:`foolysh.scene.node.Node`.

        Args:
            drag_node: :class:`foolysh.scene.node.Node`
        """
        node_id = drag_node.node_id
        if node_id in self._drag_nodes:
            self._drag_nodes.pop(node_id)
        if not self._drag_nodes:
            self._app.task_manager[self._ids[2]].pause()

    def _process(self, dt: float):
        """Process active drag operations."""
        if self._info.active > -1:
            if self._info.last_mouse is None:
                self._info.last_mouse = self._app.mouse_pos
            delta = self._app.mouse_pos - self._info.last_mouse
            drag_node = self._drag_nodes[self._info.active].node
            if delta.length / dt > MAX_SPEED:
                drag_node.pos = self._info.start_pos
                self._info.active = -1
            else:
                drag_node.pos = drag_node, delta
        self._info.last_mouse = self._app.mouse_pos

    def _mouse_down(self, event: sdl2.SDL_Event) -> None:
        """
        Drag event callback.
        """
        self._info.last_mouse = self._app.mouse_pos
        if self._info.active == -1 and (self._watch_button is None or \
             event.button.button == self._watch_button):
            mouse_pos = self._app.mouse_pos + self._app.renderer.view_pos
            mouse_aabb = aabb.AABB(mouse_pos.x, mouse_pos.y, 0, 0)
            click_node = None
            d_max = None
            for k in self._drag_nodes:
                d_node = self._drag_nodes[k]
                if d_node.node.hidden:
                    continue
                if d_node.node.aabb.overlap(mouse_aabb):
                    d_depth = d_node.node.relative_depth
                    if click_node is None or d_depth > d_max:
                        click_node = d_node
                        d_max = d_depth
            if click_node is not None:
                if click_node.drag_callback is not None:
                    res = click_node.drag_callback(*click_node.drag_args)
                    if res is False:
                        return
                self._info.active = click_node.node.node_id
                self._info.start_pos = click_node.node.pos
                return

    def _mouse_up(self, event: sdl2.SDL_Event) -> None:
        """
        Drop event callback.
        """
        if not self._app.isandroid and \
             event.button.button != self._watch_button:
            return
        if self._info.active > -1:
            drag_node = self._drag_nodes[self._info.active]
            drag_len = (self._info.start_pos - drag_node.node.pos).length
            if drag_len >= self._drag_threshold:
                if drag_node.drop_callback is not None:
                    drag_node.drop_callback(*drag_node.drop_args)
            else:
                drag_node.node.pos = self._info.start_pos
            self._info.active = -1
