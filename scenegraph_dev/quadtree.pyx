# distutils: language = c++
"""
Simple Quadtree implementation, partially based on information found on:
https://bit.ly/309V7J2
"""

from _quadtree cimport Quadtree as _Quadtree
from _nodepath cimport SmallList
from _aabb cimport AABB
from aabb cimport AABB as AABB_

from cython.operator cimport dereference as deref

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


cdef class Quadtree:
    cdef unique_ptr[_Quadtree] thisptr

    def __cinit__(self, *args, **kwargs):
        pass

    cdef bint insert(self, const int id, AABB& aabb):
        return deref(self.thisptr).insert(id, aabb)

    cdef bint move(self, const int id, AABB& aabb_from, AABB& aabb_to):
        return deref(self.thisptr).move(id, aabb_from, aabb_to)

    cdef bint remove(self, const int id, AABB& aabb):
        return deref(self.thisptr).remove(id, aabb)

    cdef bint cleanup(self):
        return deref(self.thisptr).cleanup()

    cdef bint inside(self, const double x, const double y):
        return deref(self.thisptr).inside(x, y)

    cdef void resize(self, AABB& aabb):
        deref(self.thisptr).resize(aabb)


def get_quadtree(aabb, max_leaf_elements=8, max_depth=8):
    return _get_quadtree(aabb, max_leaf_elements, max_depth)

cdef Quadtree _get_quadtree(AABB_ aabb, const int max_leaf_elements, const int max_depth):
    qt = Quadtree()
    qt.thisptr.reset(new _Quadtree(aabb.aabb(), max_leaf_elements, max_depth))
    return qt
