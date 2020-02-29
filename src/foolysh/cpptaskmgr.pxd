# distutils: language = c++
"""
Provides a simplistic Task Manager to execute either every frame or in a
specified interval.
"""

from libcpp.string cimport string
from libcpp cimport bool

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


cdef extern from "src/taskmgr.cpp" namespace "foolysh::tools":
    pass

cdef extern from "src/taskmgr.hpp" namespace "foolysh::tools":
    ctypedef void (*callback)(void*, void*, void*, const double, const bool)

    cdef cppclass TaskManager:
        TaskManager() except +
        void set_callback(callback)
        void add_task(string, const double, const bool, void*, void*, void*)
        void remove_task(string)
        void execute()
        void set_delay(string, const double)
        void pause(string)
        void resume(string)
        bool state(string)
        double get_delay(string)
