"""
Provides the Entry class for building UI components.
"""

from typing import Callable, Dict, Optional, Tuple

from . import label
from . import uihandler
from ..tools.common import COLOR
from ..tools.sdf import framed_box_str

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


class Entry(label.Label):
    """
    Provides a text entry field. It does not on its own prevent text to go
    outside the underlying frame.

    Args:
        name: Optional[``str``] -> Optional name of the text entry field.
        blink_rate: Optional[``float``] -> Duration of cursor on/off in seconds.
        hint_text: Optional[``str``] -> Text displayed when the text entry field
            is empty, defaults to an empty string.
        hint_text_color: ``Optional[COLOR]`` -> color of the hint text.
        masked: ``Optional[str]`` -> If set to a single character, text input.
            will appear masked.
        **kwargs: See :class:`~foolysh.ui.label.Label` for available keyword
            arguments.
    """
    # pylint: disable=too-many-instance-attributes
    def __init__(self, name: Optional[str] = 'Unnamed Entry',
                 blink_rate: Optional[float] = 0.5,
                 hint_text: Optional[str] = '',
                 hint_text_color: Optional[COLOR] = (160, 160, 160),
                 masked: str = '', **kwargs) -> None:
        if 'font' not in kwargs:
            raise TypeError('Expected "font" in arguments.')
        if len(masked) > 1:
            raise ValueError('Expected single character for argument masked.')
        kwargs['name'] = name
        super().__init__(**kwargs)
        if masked:
            self._display_text = self.attach_text_node(**kwargs)
            self.text_color = 0, 0, 0, 0  # Hide the labels text
        else:
            self._display_text = self._txt_node
        self._mask = masked
        kwargs['text'] = hint_text
        kwargs['text_color'] = hint_text_color
        self._cursor = self.attach_image_node(f'{name}_Cursor')
        self._cursor_blink = blink_rate
        self._setup_cursor()
        self._hint_text = self.attach_text_node(**kwargs)
        self._hint_visible = True
        self.__cb: Dict[uihandler.EventType, Tuple[Callable, Tuple, Dict]] = {}
        self._register_input_cb()

    def oninput(self, input_cb: Callable, *args, **kwargs) -> None:
        """
        Set the oninput callback (including backspace presses that cause the
        text to change).

        Args:
            input_cb: The method/function to call when an input occurs.
            *args: Positional arguments to be passed to the callback.
            *kwargs: Keyword arguments to be passed to the callback.
        """
        self.__cb[uihandler.EventType.INPUT] = input_cb, args, kwargs

    def onenter(self, enter_cb: Callable, *args, **kwargs) -> None:
        """
        Set the onenter callback.

        Args:
            enter_cb: The method/function to call when the return key is
                pressed.
            *args: Positional arguments to be passed to the callback.
            *kwargs: Keyword arguments to be passed to the callback.
        """
        self.__cb[uihandler.EventType.RETURN] = enter_cb, args, kwargs

    def onenterfocus(self, enterfocus_cb: Callable, *args, **kwargs) -> None:
        """
        Set the onenterfocus callback.

        Args:
            enterfocus_cb: The method/function to call when the text entry field
                receives focus.
            *args: Positional arguments to be passed to the callback.
            *kwargs: Keyword arguments to be passed to the callback.
        """
        self.__cb[uihandler.EventType.ENTER_FOCUS] = enterfocus_cb, args, kwargs

    def onexitfocus(self, exitfocus_cb: Callable, *args, **kwargs) -> None:
        """
        Set the onexitfocus callback.

        Args:
            exitfocus_cb: The method/function to call when the text entry field
                receives focus.
            *args: Positional arguments to be passed to the callback.
            *kwargs: Keyword arguments to be passed to the callback.
        """
        self.__cb[uihandler.EventType.EXIT_FOCUS] = exitfocus_cb, args, kwargs

    def _setup_cursor(self) -> None:
        height = self.size[1] - 2 * self.border_thickness
        height *= 0.95
        height = min(height, self.font_size * 1.1)
        width = height / 9
        sdfstr = framed_box_str(width, height, border_thickness=0,
                                frame_color=self.text_color, alpha=255)
        self._cursor.add_image(sdfstr)
        self._cursor.hide()

    def _register_input_cb(self) -> None:
        self.ui_handler.add_node(self)
        self.ui_handler.register_event(self, uihandler.EventType.INPUT,
                                       self._input_event)
        self.ui_handler.register_event(self, uihandler.EventType.BACKSPACE,
                                       self._input_event, '\b')
        self.ui_handler.register_event(self, uihandler.EventType.RETURN,
                                       self._other_event,
                                       uihandler.EventType.RETURN)
        self.ui_handler.register_event(self, uihandler.EventType.ENTER_FOCUS,
                                       self._other_event,
                                       uihandler.EventType.ENTER_FOCUS)
        self.ui_handler.register_event(self, uihandler.EventType.EXIT_FOCUS,
                                       self._other_event,
                                       uihandler.EventType.EXIT_FOCUS)
        self.ui_handler.register_event(self, uihandler.EventType.BLINK,
                                       self._blink)

    def _input_event(self, text: str) -> None:
        if text == '\b':
            if not self.text:
                return
            if len(self.text) > 1:
                self.text = self.text[:-1]
            else:
                self.text = ''
                if not self._hint_visible:
                    self._hint_text.show()
                    self._cursor.hide()
                    self._hint_visible = True
        else:
            if self._hint_visible:
                self._hint_text.hide()
                self._hint_visible = False
                self._cursor.show()
            self.text += text
        if self._mask:
            self._display_text.text = self._mask * len(self.text)
        else:
            self._display_text.text = self.text
        self._other_event(uihandler.EventType.INPUT)
        self.dirty = True
        self.ui_handler.need_render = True

    def _other_event(self, event_t: uihandler.EventType) -> None:
        if event_t in self.__cb:
            func, args, kwargs = self.__cb[event_t]
            func(*args, **kwargs)
            self.dirty = True
            self.ui_handler.need_render = True

    def _blink(self, dt, frame_time):  # pylint: disable=unused-argument
        if (frame_time / self._cursor_blink) % 2 >= 1 and self.focus:
            self._cursor.show()
        else:
            self._cursor.hide()
        self.dirty = True
        self.ui_handler.need_render = True

    def _update(self) -> None:
        if self.text and self._hint_visible:
            self._hint_visible = False
            self._hint_text.hide()
        if len(self.text) != len(self._display_text.text):
            if self._mask:
                self._display_text.text = self._mask * len(self.text)
            else:
                self._display_text.text = self.text
        if not self.text:
            self._hint_visible = True
            self._hint_text.show()
        if self._hint_visible:
            self._place_txt_node(self._hint_text)
            y = (self.size[1] - self._cursor.size[1]) / 2
            self._cursor.pos = self._hint_text.pos[0], y
        elif self._cursor.size != (0, 0):
            txtlen = len(self.text)
            self._place_txt_node(self._display_text)
            if txtlen:
                x = self._display_text.size[0] + self._display_text.size[0] \
                    / txtlen / 4
            else:
                x = 0
            x = self._display_text.x + x + self._margin
            y = (self.size[1] - self._cursor.size[1]) / 2
            self._cursor.pos = x, y
        super()._update()

    def _on_origin_change(self):
        super()._on_origin_change()
        self._cursor.origin = self.origin
        self._hint_text.origin = self.origin
