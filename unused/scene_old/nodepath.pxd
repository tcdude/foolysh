"""
Cython implementation of NodePath class.
"""
from ..tools.quadtree cimport Quadtree

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

cpdef enum Origin:
    CENTER = 0
    TOP_LEFT = 1
    BOTTOM_RIGHT = 2

ctypedef Point

cdef class NodePath:
    cdef readonly str _np_name
    cdef readonly Origin _center
    cdef readonly bint _visible
    cdef public bint _dirty
    cdef readonly bint _is_root
    cdef readonly Quadtree _quadtree
    cdef readonly Point _position
    cdef readonly Point _rel_position
    cdef readonly double _angle
    cdef readonly double _scale
    cdef readonly int _depth
    cdef readonly tuple _rotation_center
    cdef readonly double _rel_angle
    cdef readonly double _rel_scale
    cdef readonly int _rel_depth
    cdef readonly int _asset_pixel_ratio
    cdef readonly tuple _dummy_size
    cdef readonly object _node
    cdef readonly list _children
    cdef readonly dict _tags
    cdef readonly int _max_level
    cdef readonly NodePath _parent
    cdef readonly object _sprite_loader

    cdef void propagate_dirty(self)
    cpdef void set_dummy_size(self, size)
    cpdef tuple update_relative(self)
    cdef tuple _update_relative(self)
    cpdef bint traverse(self)
    cdef bint _traverse(self)
    cpdef bint reparent_to(self, NodePath new_parent)
    cpdef NodePath attach_new_node_path(self, name=*, center=*, visible=*,
                                        position=*, angle=*, scale=*, depth=*)
    cpdef list query(self, q_aabb, bint overlap=*)
    cpdef bint remove_node_path(self, NodePath np)
    cpdef pop(self, item)
