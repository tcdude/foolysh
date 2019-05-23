# distutils: language = c++

from aabb cimport AABB

from cython.operator cimport dereference as deref

from libcpp.memory cimport unique_ptr


# Create a Cython extension type which holds a C++ instance
# as an attribute and create a bunch of forwarding methods
# Python extension type.
cdef class PyAABB:
    cdef unique_ptr[AABB] thisptr

    def __cinit__(self, double x0, double y0, double x1, double y1):
        self.thisptr.reset(new AABB(x0, y0, x1, y1))

    def inside_aabb(self, other):
        return self._inside_aabb(other)

    cdef bint _inside_aabb(self, PyAABB other):
        return deref(self.thisptr).inside(other.aabb())

    def inside_tup(self, x, y):
        return deref(self.thisptr).inside(<double>x, <double>y)

    def overlap(self, other):
        return self._overlap(other)

    cdef bint _overlap(self, PyAABB other):
        return deref(self.thisptr).overlap(other.aabb())

    cdef AABB aabb(self):
        return deref(self.thisptr)
