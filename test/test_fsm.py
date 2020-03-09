"""
Unittests for foolysh.fsm
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


class MyStateA(fsm.FSM):
    def __init__(self):
        super().__init__()
        self.enter = 0
        self.exit = 0

    def enter_my_state_a(self):
        self.enter += 1

    def exit_my_state_a(self):
        self.exit += 1


class MyStateB(fsm.FSM):
    def __init__(self):
        super().__init__()
        self.enter = 0
        self.exit = 0

    def enter_my_state_b(self):
        self.enter += 1

    def exit_my_state_b(self):
        self.exit += 1


def test_fsm():
    a = MyStateA()
    b = MyStateB()
    a.request('my_state_a')
    assert a.enter == 1
    a.request('my_state_b')
    assert a.exit == 1
    assert b.enter == 1
    b.request('my_state_b')
    assert b.enter == 1
    assert b.exit == 0
    b.request('my_state_a')
    assert b.exit == 1
    assert a.enter == 2
