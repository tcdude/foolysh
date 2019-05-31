# distutils: language = c++
"""
Basic 2D Vector implementation.
"""

from .cppvector2 cimport Vector2 as _Vector2

from libcpp.memory cimport unique_ptr

__author__ = 'Tiziano Bettio'
__license__ = 'MIT'
__version__ = '0.1'
__copyright__ = """Copyright (c) 2019 Tiziano Bettio

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


cdef class Vector2:
    cdef unique_ptr[_Vector2] thisptr

    cdef double _dot(self, Vector2 other)
    cdef Vector2 _normalized(self)
    cpdef void rotate(self, double a, bint radians=*)
    cpdef Vector2 rotated(self, double a, bint radians=*)
    cpdef Vector2 add(self, Vector2 other)
    cdef void _iadd_vec(Vector2 self, Vector2 other)
    cpdef Vector2 add_scalar(self, const double other)
    cpdef Vector2 sub(self, Vector2 other)
    cdef void _isub_vec(Vector2 self, Vector2 other)
    cpdef Vector2 sub_scalar(self, const double other)
    cpdef Vector2 sub_scalar_r(self, const double other)
    cpdef Vector2 neg(self)
    cpdef Vector2 mul(self, const double other)
    cpdef Vector2 tdiv(self, double value)
    cpdef bint eq(self, Vector2 other)
    cpdef bint eq_scalar(self, double other)
    cpdef bint ne(self, Vector2 other)
    cpdef bint ne_scalar(self, double other)
    cdef public _Vector2 vector2(self)
