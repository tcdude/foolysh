"""
Unittests for the foolysh.fsm module.
"""

from foolysh import fsm

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


class App(fsm.FSM):
    """A mock up of the App class that usually inherits from FSM."""


class MyStateA(App):
    """A class representing a possible state."""
    def __init__(self):
        super().__init__()
        self.enter_a = 0
        self.exit_a = 0

    def enter_my_state_a(self):
        """Enter method that gets called when this state is requested."""
        self.enter_a += 1

    def exit_my_state_a(self):
        """Exit method that gets called when this state is left."""
        self.exit_a += 1


class MyStateB(App):
    """A class representing a possible state."""
    def __init__(self):
        super().__init__()
        self.enter_b = 0
        self.exit_b = 0

    def enter_my_state_b(self):
        """Enter method that gets called when this state is requested."""
        self.enter_b += 1

    def exit_my_state_b(self):
        """Exit method that gets called when this state is left."""
        self.exit_b += 1


class MyApp(MyStateA, MyStateB):
    """Inherit from all states."""


def test_fsm():
    """Test whether state changes work as expected."""
    myapp = MyApp()
    myapp.request('my_state_a')
    assert myapp.enter_a == 1
    myapp.request('my_state_b')
    assert myapp.exit_a == 1
    assert myapp.enter_b == 1
    myapp.request('my_state_b')
    assert myapp.enter_b == 1
    assert myapp.exit_b == 0
    myapp.request('my_state_a')
    assert myapp.exit_b == 1
    assert myapp.enter_a == 2
