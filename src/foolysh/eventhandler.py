"""
Provides a simplistic EventHandler class to handle SDL2 Events.
"""

import sdl2
import sdl2.ext

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


class EventHandler:
    """
    Basic EventHandler to coordinate SDL2 Events.
    """
    def __init__(self):
        self._events = {}
        self._unique = {}
        self._last_text_input = None

    def listen(self, name, sdl_event, callback, *args, **kwargs):
        """
        Adds a callback to be executed at every call of the EventHandler. An
        optional `priority` can be specified as keyword argument, to set a
        priority for this event (default=0), higher priority gets called first.

        Args:
            name: Unique name of the event
            sdl_event: SDL event type tested against ``sdl2.ext.get_events()``
            callback: Method to execute. Must provide ``event`` as named
                argument.
            args: optional positional arguments to pass to ``callback``.
            kwargs: optional keyword arguments to pass to ``callback``.

        .. warn::
            The `priority` keyword argument, if specified, will be filtered out
            of the keyword arguments that get passed on to your callback!
            Do not use a keyword argument named `priority` in the signature of
            the provided `callback`!
        """
        if name in self._unique:
            raise ValueError('An event with this name already exists.')
        if sdl_event not in self._events:
            self._events[sdl_event] = {}
        if 'priority' in kwargs:
            self._events[sdl_event][name] = kwargs.pop('priority')
        else:
            self._events[sdl_event][name] = 0
        self._unique[name] = (callback, args, kwargs)

    def forget(self, name):
        """
        Removes event from the EventHandler.

        Args:
            name: ``str`` unique name of the event to be removed.
        """
        pop_list = []
        for event in self._events:
            if name in self._events[event]:
                self._events[event].pop(name)
            if not self._events[event]:
                pop_list.append(event)
        for event in pop_list:
            self._events.pop(event)
        if name in self._unique:
            self._unique.pop(name)

    @property
    def last_text_input(self):
        """
        ``str`` or ``None`` -> If a SDL_TEXTINPUT event was registered during
        the last call to the :class:`EventHandler`, the text from that event is
        returned, otherwise ``None``.
        """
        return self._last_text_input

    def __call__(self, *unused_args, **unused_kwargs):
        """
        Check and execute events.
        """
        self._last_text_input = None
        for event in sdl2.ext.get_events():
            if event.type in self._events:
                self._exec_event(event)
            if event.type == sdl2.SDL_TEXTINPUT:
                self._last_text_input = event.text.text.decode('utf-8')

    def _exec_event(self, event):
        k = sorted(self._events[event.type],
                   key=lambda x: self._events[event.type][x], reverse=True)
        for name in k:
            meth, args, kwargs = self._unique[name]
            kwargs['event'] = event
            meth(*args, **kwargs)
