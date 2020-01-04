# distutils: language = c++
"""
Simple Quadtree implementation, partially based on information found on:
https://bit.ly/309V7J2
"""

from .cppquadtree cimport Quadtree as _Quadtree
from ..scene.cppnode cimport SmallList
from .cppaabb cimport AABB as _AABB
from .aabb cimport AABB

from cython.operator cimport dereference as deref

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


cdef class Quadtree:
    cdef unique_ptr[_Quadtree] thisptr

    def __cinit__(self, *args, **kwargs):
        pass

    def __init__(self, aabb=None, max_leaf_elements=8, max_depth=8):
        if aabb is None:
            aabb = AABB()
        self._init_ptr(aabb, max_leaf_elements, max_depth)

    cdef _init_ptr(self, AABB aabb, const int mle, const int md):
        cdef _AABB caabb = aabb.aabb()
        self.thisptr.reset(new _Quadtree(caabb, mle, md))

    cpdef bint insert(self, const int id, AABB aabb):
        cdef _AABB caabb = aabb.aabb()
        return deref(self.thisptr).insert(id, caabb)

    cpdef bint move(self, const int id, AABB aabb_from, AABB aabb_to):
        cdef _AABB caabb_f = aabb_from.aabb()
        cdef _AABB caabb_t = aabb_to.aabb()
        return deref(self.thisptr).move(id, caabb_f, caabb_t)

    cpdef bint remove(self, const int id, AABB aabb):
        cdef _AABB caabb = aabb.aabb()
        return deref(self.thisptr).remove(id, caabb)

    cpdef bint cleanup(self):
        return deref(self.thisptr).cleanup()

    cpdef bint inside(self, const double x, const double y):
        return deref(self.thisptr).inside(x, y)

    cpdef void resize(self, AABB aabb):
        cdef _AABB caabb = aabb.aabb()
        deref(self.thisptr).resize(caabb)

    cpdef list query(self, AABB aabb):
        cdef _AABB caabb = aabb.aabb()
        cdef SmallList[int] r = deref(self.thisptr).query(caabb)
        cdef list rl = []
        for i in range(r.size()):
            rl.append(r[i])
        return rl
