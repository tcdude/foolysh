# distutils: language = c++
"""
Basic 2D Vector implementation.
"""

from .cppvec2 cimport Vec2 as _Vec2

from libcpp.memory cimport unique_ptr

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


cdef class Vec2:
    cdef unique_ptr[_Vec2] thisptr

    cdef double _dot(self, Vec2 other)
    cdef Vec2 _normalized(self) except +
    cpdef void rotate(self, double a, bint radians=*)
    cpdef Vec2 rotated(self, double a, bint radians=*)
    cpdef Vec2 _add(self, Vec2 other)
    cdef void _iadd_vec(Vec2 self, Vec2 other)
    cpdef Vec2 _add_scalar(self, const double other)
    cpdef Vec2 _sub(self, Vec2 other)
    cdef void _isub_vec(Vec2 self, Vec2 other)
    cpdef Vec2 _sub_scalar(self, const double other)
    cpdef Vec2 _sub_scalar_r(self, const double other)
    cpdef Vec2 _neg(self)
    cpdef Vec2 _mul(self, const double other)
    cpdef Vec2 _tdiv(self, double value)
    cpdef bint _eq(self, Vec2 other)
    cpdef bint _eq_scalar(self, double other)
    cpdef bint _ne(self, Vec2 other)
    cpdef bint _ne_scalar(self, double other)
    cdef public _Vec2 vec2(self)
