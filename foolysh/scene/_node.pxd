# distutils: language = c++

from .cppnode cimport Node as _Node
from .cppnode cimport SceneGraphDataHandler as _SceneGraphDataHandler
from .cppnode cimport Origin as _Origin
from ..tools.aabb cimport AABB
from ..tools.vector2 cimport Vector2

from libcpp.memory cimport unique_ptr


cdef class SceneGraphDataHandler:
    cdef unique_ptr[_SceneGraphDataHandler] thisptr


cdef class Node:
    cdef unique_ptr[_Node] thisptr
    cdef str __name

    cdef void _setup(self, SceneGraphDataHandler sgdh)

    cpdef void remove(self)
    cdef void _attach_node(self, Node np)
    cdef void _reparent_to(self, Node parent) except +
    cdef list _query(self, AABB aabb, bint depth_sorted) except +
    cdef Vector2 _get_pos(self)
    cdef void _set_pos_single(self, const double v)
    cdef void _set_pos(self, const double x, const double y)
    cdef void _set_pos_relative(
        self,
        Node other,
        const double x,
        const double y
    )
    cdef Vector2 _get_pos_node(self, Node other)
    cdef tuple _get_scale(self)
    cdef tuple _get_scale_node(self, Node other)
    cdef void _set_scale_single(self, const double v)
    cdef void _set_scale(self, const double x, const double y)
    cdef void _set_scale_node(self, Node other, const double x, const double y)
    cdef double _get_angle_node(self, Node other)
    cdef void _set_angle(self, const double d)
    cdef void _set_angle_node(self, Node other, const double r)
    cdef double _get_angle_rad_node(self, Node other)
    cdef void _set_angle_rad(self, const double r)
    cdef void _set_angle_rad_node(self, Node other, const double r)
    cdef Vector2 _get_rotation_center(self)
    cdef void _set_rotation_center(self, const double x, const double y)
    cdef void _set_depth_node(self, Node other, int depth)
    cdef void _set_origin(self, _Origin o)
    cdef Vector2 _get_relative_pos(self)
    cdef tuple _get_relative_scale(self)
    cdef tuple _get_size(self)
    cdef void _set_size(self, const double x, const double y)
    cdef AABB _get_aabb(self)
    cdef tuple _get_relative_size(self)
