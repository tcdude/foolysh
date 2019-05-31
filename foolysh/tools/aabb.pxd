# distutils: language = c++

from .cppaabb cimport AABB as _AABB
from .cppaabb cimport Quadrant

from libcpp.memory cimport unique_ptr


cdef class AABB:
    cdef unique_ptr[_AABB] thisptr

    cdef bint _inside_aabb(self, AABB other)
    cdef bint _overlap(self, AABB other)
    cdef _AABB _split(self, Quadrant q)
    cdef _AABB _split_point(self, double x, double y, Quadrant q)
    cdef public _AABB aabb(self)
