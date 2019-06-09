# distutils: language = c++
"""
Node implementation.
"""

from enum import Enum

from .cppnode cimport Node as _Node
from .cppnode cimport SmallList
from .cppnode cimport Scale
from .cppnode cimport Size
from .cppnode cimport Origin as _Origin
from ..tools.cppaabb cimport AABB as _AABB
from ..tools.aabb cimport AABB
from ..tools.cppvector2 cimport Vector2 as _Vector2
from ..tools.vector2 cimport Vector2

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

cdef dict _nodepaths = {}


class Origin(Enum):
    TOP_LEFT=0
    TOP_RIGHT=1
    TOP_CENTER=2
    CENTER_LEFT=3
    CENTER=4
    CENTER_RIGHT=5
    BOTTOM_LEFT=6
    BOTTOM_RIGHT=7
    BOTTOM_CENTER=8


cdef class Node:
    cdef unique_ptr[_Node] thisptr

    def __cinit__(self):
        self.thisptr.reset(new _Node())
        # Reference the Node instance to keep it alive
        _nodepaths[deref(self.thisptr).get_id()] = self

    @property
    def node_id(self):
        return deref(self.thisptr).get_id()

    cpdef void remove(self):
        """Removes Node so it can get garbage collected."""
        _nodepaths.pop(deref(self.thisptr).get_id())

    def attach_node(self):
        np = Node.__new__(Node)
        self._attach_node(np)
        return np

    cdef void _attach_node(self, Node np):
        cdef unique_ptr[_Node] _np
        _nodepaths.pop(deref(np.thisptr).get_id())
        _np.reset(new _Node(deref(self.thisptr).attach_node()))
        _nodepaths[deref(_np).get_id()] = np
        np.thisptr.reset(_np.release())

    def reparent_to(self, parent):
        self._reparent_to(parent)

    cdef void _reparent_to(self, Node parent):
        deref(self.thisptr).reparent_to(deref(parent.thisptr))

    def traverse(self):
        return deref(self.thisptr).traverse()

    def query(self, aabb):
        return self._query(aabb)

    cdef list _query(self, AABB aabb):
        cdef SmallList[int] r = deref(self.thisptr).query(deref(aabb.thisptr))
        cdef list rl = []
        for i in range(r.size()):
            rl.append(_nodepaths[r[i]])
        return rl

    def hide(self):
        deref(self.thisptr).hide()

    def show(self):
        deref(self.thisptr).show()

    @property
    def pos(self):
        return self._get_pos()

    @pos.setter
    def pos(self, v):
        if isinstance(v, Vector2):
            self._set_pos(v.x, v.y)
        elif isinstance(v, (int, float)):
            self._set_pos_single(v)
        elif isinstance(v, tuple) and isinstance(v[0], (int, float)) \
             and isinstance(v[1], (int, float)):
            self._set_pos(v[0], v[1])
        else:
            raise TypeError

    cdef Vector2 _get_pos(self):
        cdef _Vector2 v = deref(self.thisptr).get_pos()
        return Vector2(v[0], v[1])

    cdef void _set_pos_single(self, const double v):
        deref(self.thisptr).set_pos(v)

    cdef void _set_pos(self, const double x, const double y):
        deref(self.thisptr).set_pos(x, y)

    @property
    def scale(self):
        s = self._get_scale()
        if s[0] == s[1]:
            return s[0]
        return s

    @scale.setter
    def scale(self, v):
        if isinstance(v, (int, float)):
            self._set_scale_single(v)
        elif isinstance(v, tuple) and isinstance(v[0], (int, float)) \
             and isinstance(v[1], (int, float)):
            self._set_scale(v[0], v[1])
        else:
            raise TypeError

    cdef tuple _get_scale(self):
        cdef Scale s = deref(self.thisptr).get_scale()
        return s.sx, s.sy

    cdef void _set_scale_single(self, const double v):
        deref(self.thisptr).set_scale(v)

    cdef void _set_scale(self, const double x, const double y):
        deref(self.thisptr).set_scale(x, y)

    @property
    def angle(self):
        return deref(self.thisptr).get_angle(False)

    @angle.setter
    def angle(self, v):
        if isinstance(v, (int, float)):
            self._set_angle(v)
        else:
            raise TypeError

    cdef void _set_angle(self, const double d):
        deref(self.thisptr).set_angle(d, False)

    @property
    def angle_rad(self):
        return deref(self.thisptr).get_angle(True)

    @angle_rad.setter
    def angle_rad(self, v):
        if isinstance(v, (int, float)):
            self._set_angle_rad(v)
        else:
            raise TypeError

    cdef void _set_angle_rad(self, const double r):
        deref(self.thisptr).set_angle(r, True)

    @property
    def rotation_center(self):
        return self._get_rotation_center()

    @rotation_center.setter
    def rotation_center(self, v):
        if isinstance(v, Vector2):
            self._set_rotation_center(v.x, v.y)
        elif v == 0:
            self._set_rotation_center(0.0, 0.0)
        elif isinstance(v, tuple) and isinstance(v[0], (int, float)) \
             and isinstance(v[1], (int, float)):
            self._set_rotation_center(v[0], v[1])
        else:
            raise TypeError

    cdef Vector2 _get_rotation_center(self):
        cdef _Vector2 v = deref(self.thisptr).get_rotation_center()
        return Vector2(v[0], v[1])

    cdef void _set_rotation_center(self, const double x, const double y):
        deref(self.thisptr).set_rotation_center(x, y)

    @property
    def depth(self):
        return deref(self.thisptr).get_depth()

    @depth.setter
    def depth(self, v):
        if isinstance(v, int):
            deref(self.thisptr).set_depth(v)
        else:
            raise TypeError

    @property
    def origin(self):
        return Origin(deref(self.thisptr).get_origin())

    @origin.setter
    def origin(self, v):
        if isinstance(v, Origin):
            self._set_origin(v.value)
        else:
            raise TypeError

    cdef void _set_origin(self, _Origin o):
        deref(self.thisptr).set_origin(o)

    @property
    def relative_pos(self):
        return self._get_relative_pos()

    cdef Vector2 _get_relative_pos(self):
        cdef _Vector2 v = deref(self.thisptr).get_relative_pos()
        return Vector2(v[0], v[1])

    @property
    def relative_scale(self):
        s = self._get_relative_scale()
        if s[0] == s[1]:
            return s[0]
        return s

    cdef tuple _get_relative_scale(self):
        cdef Scale s = deref(self.thisptr).get_relative_scale()
        return s.sx, s.sy

    @property
    def relative_angle(self):
        return deref(self.thisptr).get_relative_angle()

    @property
    def relative_depth(self):
        return deref(self.thisptr).get_relative_depth()

    @property
    def size(self):
        s = self._get_size()
        return s

    @size.setter
    def size(self, v):
        if isinstance(v, tuple) and isinstance(v[0], (int, float)) \
             and isinstance(v[1], (int, float)):
            self._set_size(v[0], v[1])
        else:
            raise TypeError

    cdef tuple _get_size(self):
        cdef Size s = deref(self.thisptr).get_size()
        return s.w, s.h

    cdef void _set_size(self, const double x, const double y):
        deref(self.thisptr).set_size(x, y)

    @property
    def distance_relative(self):
        return deref(self.thisptr).get_distance_relative()

    @distance_relative.setter
    def distance_relative(self, v):
        if isinstance(v, bool):
            deref(self.thisptr).set_distance_relative(v)
        else:
            raise TypeError

    @property
    def relative_size(self):
        s = self._get_relative_size()
        return s

    @property
    def aabb(self):
        return self._get_aabb()

    cdef AABB _get_aabb(self):
        cdef _AABB _aabb = deref(self.thisptr).get_aabb()
        return AABB(_aabb.x, _aabb.y, _aabb.hw, _aabb.hh)

    cdef tuple _get_relative_size(self):
        cdef Size s = deref(self.thisptr).get_relative_size()
        return s.w, s.h

    def __repr__(self):
        return f'{type(self).__name__}{str(self)}'

    def __str__(self):
        return f'(id:{self.node_id})'
