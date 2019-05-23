# distutils: language = c++

cdef extern from "src/aabb.cpp":
    pass

cdef extern from "src/aabb.hpp" namespace "scenegraph":
    cdef cppclass AABB:
        AABB() except +
        AABB(double, double, double, double) except +
        bint inside(AABB)
        bint inside(double, double)
        bint overlap(AABB)
        double x0, y0, x1, y1
