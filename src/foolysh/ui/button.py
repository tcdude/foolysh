"""
Provides various Button classes for building UI components.
"""

from dataclasses import dataclass
from typing import Callable, Dict, Optional, Tuple, Union

from . import label
from . import uihandler
from . import uinode
from . import UIState

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


@dataclass
class ButtonLabel:
    """The labels attached to the button."""
    normal: label.Label
    down: label.Label
    disabled: label.Label

    def __len__(self):
        return 3

    def __getitem__(self, item: Union[int, slice, UIState]) -> label.Label:
        if isinstance(item, UIState):
            if item == UIState.NORMAL:
                return self.normal
            if item == UIState.ACTIVE:
                return self.down
            if item == UIState.DISABLED:
                return self.disabled
            raise IndexError(f'Unknown state {repr(item)}')
        return [self.normal, self.down, self.disabled][item]


class Button(uinode.UINode):
    """
    A Button is basically a collection of three
    :class:`~foolysh.ui.label.Label`s with only keyword arguments allowed.
    Keyword arguments without prefix represent the default/normal state of the
    Button, while keyword arguments prefixed with one of "down_" or "disabled_"
    represent the respective state.

    Args:
        **kwargs: Any keyword argument allowed in
            :class:`~foolysh.ui.frame.Frame` and
            :class:`~foolysh.ui.label.Label`. Plus the same, but with
            appropriate prefix for different states.
    """
    def __init__(self, name: Optional[str] = 'Unnamed Button',
                 size: Optional[Tuple[float, float]] = (0.0, 0.0),
                 pos: Union[Tuple[float, float], vec2.Vec2] = vec2.Vec2(),
                 **kwargs) -> None:
        if size == (0.0, 0.0):
            raise ValueError('Expected size different to 0, 0.')
        super().__init__(name, size, pos)
        kwargs['name'] = f'Button Label {name}'
        kwargs['size'] = size
        kwargs['pos'] = 0, 0
        down_kw = {}
        disabled_kw = {}
        down_kw.update(kwargs)
        disabled_kw.update(kwargs)
        for k in kwargs:
            for dct, pre in ((down_kw, 'down_'), (disabled_kw, 'disabled_')):
                if k.startswith(pre):
                    dct[k[len(pre):]] = kwargs[k]

        self._labels = ButtonLabel(label.Label(**kwargs),
                                   label.Label(**down_kw),
                                   label.Label(**disabled_kw))
        for i in self._labels:
            i.reparent_to(self)
        for i in self._labels[1:]:
            i.hide()
        self._register_state_cb()
        self.__cb: Dict[uihandler.EventType, Tuple[Callable, Tuple, Dict]] = {}
        self.__state = UIState.NORMAL

    def onclick(self, click_cb: Callable, *args, **kwargs):
        """
        Set the onclick callback.

        Args:
            click_cb: The method/function to call when a click occurs.
            *args: Positional arguments to be passed to the callback.
            *kwargs: Keyword arguments to be passed to the callback.
        """
        self.__cb[uihandler.EventType.CLICK] = click_cb, args, kwargs

    def ondown(self, down_cb: Callable, *args, **kwargs):
        """
        Set the ondown callback. This callback fires every frame the button is
        in the down state.

        Args:
            down_cb: The method/function to call when the button is down occurs.
            *args: Positional arguments to be passed to the callback.
            *kwargs: Keyword arguments to be passed to the callback.
        """
        self.__cb[uihandler.EventType.DOWN] = down_cb, args, kwargs

    def _update(self):  # pylint: disable=useless-super-delegation
        super()._update()

    def _on_origin_change(self):
        super()._on_origin_change()
        for i in self._labels:
            i.origin = self.origin

    def _register_state_cb(self):
        self.ui_handler.add_node(self)
        self.ui_handler.register_event(self, uihandler.EventType.DOWN,
                                       self._on_down)
        self.ui_handler.register_event(self, uihandler.EventType.CLICK,
                                       self._on_click)
        self.ui_handler.register_event(self, uihandler.EventType.RESET,
                                       self._on_reset)

    def _on_down(self):
        if self.__state == UIState.DISABLED:
            return
        if self.__state != UIState.ACTIVE:
            self._change_state(UIState.ACTIVE)
        if uihandler.EventType.DOWN in self.__cb:
            func, args, kwargs = self.__cb[uihandler.EventType.DOWN]
            func(*args, **kwargs)

    def _on_click(self):
        if self.__state == UIState.DISABLED:
            return
        self._change_state(UIState.NORMAL)
        if uihandler.EventType.CLICK in self.__cb:
            func, args, kwargs = self.__cb[uihandler.EventType.CLICK]
            func(*args, **kwargs)

    def _on_reset(self):
        if self.__state == UIState.DISABLED:
            return
        self._change_state(UIState.NORMAL)

    def _change_state(self, state: UIState):
        self.__state = state
        if state == UIState.ACTIVE:
            self._labels.normal.hide()
            self._labels.down.show()
            self._labels.down.dirty = True
            self._labels.disabled.hide()
        elif state == UIState.NORMAL:
            self._labels.normal.show()
            self._labels.normal.dirty = True
            self._labels.down.hide()
            self._labels.disabled.hide()
        elif state == UIState.DISABLED:
            self._labels.normal.hide()
            self._labels.down.hide()
            self._labels.disabled.show()
            self._labels.disabled.dirty = True
        else:
            raise ValueError(f'Unknown state {repr(state)}.')
        self.dirty = True

    @property
    def labels(self) -> label.Label:
        """
        Access to the attached :class:`~foolysh.ui.label.Label`s that make up
        the button. See :class:`ButtonLabel` for members.
        """
        return self._labels

    @property
    def enabled(self) -> bool:
        """
        Whether the button is be enabled.

        :setter:
            ``bool``
        """
        return self.__state != UIState.DISABLED

    @enabled.setter
    def enabled(self, value: bool) -> None:
        if not isinstance(value, bool):
            raise TypeError('Expected type bool.')
        if value == (self.__state != UIState.DISABLED):
            return
        if value:
            self._change_state(UIState.NORMAL)
        else:
            self._change_state(UIState.DISABLED)
