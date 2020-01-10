"""
Unittests for foolysh.taskmanager
"""
import time

from foolysh import taskmanager

__author__ = 'Tiziano Bettio'
__license__ = 'MIT'
__version__ = '0.2'
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


def test_taskmanager():
    def callback_no_dt(arg_a, arg_b, _=None, kwarg_a='blah'):
        assert arg_a == 'a'
        assert arg_b == 'b'
        assert kwarg_a == 'kwa'

    def callback_dt(dt):
        assert dt == 0.0
    tm = taskmanager.TaskManager()
    tm.add_task(
        'test_task_no_dt',
        callback_no_dt,
        0,
        False,
        ('a', 'b'),
        {'kwarg_a': 'kwa'}
    )
    tm.add_task('test_task_dt', 0, True, callback_dt)
    tm()


def test_timing():
    def callback(a):
        a.append(1)

    cb_counter = []
    tm = taskmanager.TaskManager()
    tm.add_task(
        'counter_task',
        callback,
        0.02,
        False,
        (cb_counter, )
    )
    start_time = time.perf_counter()
    while time.perf_counter() - start_time < 1:
        tm()
    assert sum(cb_counter) == 49


def test_pause_resume():
    def callback(a):
        a.append(1)

    cb_counter = []
    tm = taskmanager.TaskManager()
    task = tm.add_task(
        'pause_test',
        callback,
        0,
        False,
        (cb_counter, )
    )
    tm()
    assert sum(cb_counter) == 1
    task.pause()
    tm()
    assert task.ispaused is True
    assert sum(cb_counter) == 1
    task.resume()
    tm()
    assert task.ispaused is False
    assert sum(cb_counter) == 2


def test_delay_change():
    def callback(t):
        assert time.perf_counter() - t > 0.5

    tm = taskmanager.TaskManager()
    tm()
    task = tm.add_task(
        'delay_change',
        0,
        False,
        callback,
        (time.perf_counter(), )
    )
    task.delay = 0.5
    tm()
    time.sleep(0.5)
    tm()


def test_subscriptable():
    def callback():
        pass
    tm = taskmanager.TaskManager()
    task = tm.add_task(
        'subscriptable',
        0,
        False,
        callback
    )
    assert task == tm['subscriptable']
