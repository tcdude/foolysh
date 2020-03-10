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
    Rudimentary Finite State Machine to organize state changes. If both a
    `enter_[my_class_name]` and `exit_[my_class_name]` are provided in a
    subclass of FSM, it will become a state that can be activated through the
    :method:`self.request` method. `[my_class_name]` is a snake_case
    representation of the name of the subclass. It assumes PascalCase for class
    names (i.e. `MyClass` -> `my_class`). Use
    :func:`~foolysh.tools.common.to_snake_case` with your class name as
    parameter to determine the proper state name.

    When :meth:`FSM.request` is called the following actions are performed:

        * The `exit_` method from the current state gets called, if a state has
            been previously been activated.
        * The `enter_` method from the requested state gets called.

    .. info::
        Only provide `enter_` / `exit_` methods for subclasses that should be
        callable states.
    """
    __states: Dict[str, Tuple[Callable, Callable]] = {}
    __active_state: Optional[str] = None

    def __setup_fsm(self):
        mro = [i.__name__ for i in self.__class__.__mro__]
        mro = mro[:mro.index('FSM')]
        for i in mro:
            name = to_snake_case(i)
            enterm = getattr(self, f'enter_{name}', False)
            exitm = getattr(self, f'exit_{name}', False)
            if enterm and exitm:
                self.__states[name] = enterm, exitm
            else:
                Warning(f'Class "{i}" does not expose enter and exit methods. '
                        f'State not registered!')

    def request(self, state_name: str) -> None:
        """
        Performs the transition to a registered State. Raises a ValueError if
        the provided `state_name` is not registered.
        """
        if not self.__states:
            self.__setup_fsm()
        if state_name not in self.__states:
            raise ValueError(f'Unknown state "{state_name}".')
        if self.__active_state == state_name:
            return
        if self.__active_state is not None:
            self.__states[self.__active_state][1]()
        self.__states[state_name][0]()
        self.__active_state = state_name

    @property
    def active_state(self) -> str:
        """The currently active state."""
        return self.__active_state
