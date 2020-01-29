# distutils: language = c++
"""
Basic 2D Vector implementation.
"""

from .cppaabb cimport AABB as _AABB
from .cppaabb cimport Quadrant

from cython.operator cimport dereference as deref


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


cdef class AABB:
    def __cinit__(
        self,
        double x=0.0,
        double y=0.0,
        double hw=1.0,
        double hh=1.0
        ):
        self.thisptr.reset(new _AABB(x, y, hw, hh))

    def inside_aabb(self, other):
        return self._inside_aabb(other)

    cdef bint _inside_aabb(self, AABB other):
        return deref(self.thisptr).inside(other.aabb())

    def inside_tup(self, x, y):
        return deref(self.thisptr).inside(<double>x, <double>y)

    def overlap(self, other):
        return self._overlap(other)

    cdef bint _overlap(self, AABB other):
        return deref(self.thisptr).overlap(other.aabb())

    def split(self, q):
        cdef Quadrant _q = q
        cdef _AABB _aabb = self._split(_q)
        return AABB(_aabb.x, _aabb.y, _aabb.hw, _aabb.hh)

    cdef _AABB _split(self, Quadrant q):
        return deref(self.thisptr).split(q)

    def split_point(self, x, y, q):
        cdef Quadrant _q = q
        cdef _AABB _aabb = self._split_point(x, y, _q)
        return AABB(_aabb.x, _aabb.y, _aabb.hw, _aabb.hh)

    cdef _AABB _split_point(self, double x, double y, Quadrant q):
        return deref(self.thisptr).split(x, y, q)

    cdef _AABB aabb(self):
        return deref(self.thisptr)

    @property
    def pos(self):
        return deref(self.thisptr).x, deref(self.thisptr).y

    @property
    def size(self):
        return deref(self.thisptr).hw, deref(self.thisptr).hh

    def __repr__(self):
        return f'{type(self).__name__}{str(self)}'

    def __str__(self):
        cdef double x, y, hw, hh
        x = deref(self.thisptr).x
        y = deref(self.thisptr).y
        hw = deref(self.thisptr).hw
        hh = deref(self.thisptr).hh
        return f'(pos: {x:.4}, {y:.4} / sz: {hw:.4}, {hh:.4})'
