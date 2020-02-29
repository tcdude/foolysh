# distutils: language = c++
"""
Simple Quadtree implementation, partially based on information found on:
https://bit.ly/309V7J2
"""

from .cppaabb cimport AABB
from ..scene.cppnode cimport SmallList

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


cdef extern from "src/quadtree.cpp":
    pass

cdef extern from "src/quadtree.hpp" namespace "foolysh::tools":
    cdef cppclass Quadtree:
        Quadtree() except +
        Quadtree(const AABB&, const int, const int) except +

        SmallList[int] query(AABB&)
        bint insert(const int, AABB&)
        bint move(const int, AABB&, AABB&)
        bint remove(const int, AABB&)
        bint cleanup()
        bint inside(const double, const double)
        void resize(AABB&)
