"""
Provides the FSM class, a rudimentary implementation of a Finite State Machine.
"""

from typing import Callable, Dict, Optional, Tuple

from .tools.common import to_snake_case

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


class FSM:
    """
    Rudimentary Finite State Machine to organize state changes. Use this class
    to subclass from and provide both a `enter_` and `exit_[your_class_name]`
    method, where the used class name is transformed from the recommended
    PascalCase for class names to snake_case (i.e. MyClass would result in
    my_class). If unsure about what the transformed name will be, run the
    :func:`~foolysh.tools.common.to_snake_case` with your class name as
    parameter to get the proper snake_case name to use.

    The FSM class exposes the :meth:`FSM.request` method to change state. On a
    state change the following is executed:

        * The `exit_` method from the current state gets called.
        * The `enter_` method from the requested state gets called.

    The state name argument of the :meth:`FSM.request` method is the same
    aforementioned snake_case converted name of your subclass.

    .. warning::
        Only the last instantiated object of a class will be receiving enter and
        exit calls!!!
    """
    __states: Dict[str, Tuple[Callable, Callable]] = {}
    __active_state: Optional[str] = None

    def __init__(self):
        name = to_snake_case(type(self).__name__)
        if name in FSM.__states:
            Warning('The state has already been registered! This could lead to '
                    'undefined behavior.')
        enterm = f'enter_{name}'
        exitm = f'exit_{name}'
        FSM.__states[name] = getattr(self, enterm), getattr(self, exitm)

    @staticmethod
    def request(state_name: str) -> None:
        """
        Request the transition to a registered State.
        """
        if state_name not in FSM.__states:
            raise ValueError(f'Unknown state "{state_name}".')
        if FSM.__active_state == state_name:
            return
        if FSM.__active_state is not None:
            FSM.__states[FSM.__active_state][1]()
        FSM.__states[state_name][0]()
        FSM.__active_state = state_name

    @property
    def active_state(self) -> str:
        """The currently active state."""
        return FSM.__active_state
