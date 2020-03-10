# distutils: language = c++
"""
NodePath implementation.
"""

from ..tools.cppvec2 cimport Vec2
from ..tools.cppaabb cimport AABB

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

cdef extern from "src/common.hpp":
    pass

cdef extern from "src/quadtree.cpp":
    pass

cdef extern from "src/quadtree.hpp":
    pass

cdef extern from "src/list_t.hpp" namespace "foolysh::tools":
    cdef cppclass FreeList[T]:
        FreeList()
        int insert(T&)
        void erase(int)
        void clear()
        int range()
        T& operator[](int)

    cdef cppclass SmallList[T]:
        SmallList()
        void push_back(T&)
        T& pop_back()
        int size()
        T& operator[](int)

    cdef cppclass ExtFreeList[T]:
        ExtFreeList()
        int insert(T&)
        void erase(int)
        void clear()
        int range()
        bint active(int)
        T& operator[](int)

cdef extern from "src/node.cpp":
    pass

cdef extern from "src/node.hpp" namespace "foolysh::scene":
    cdef enum Origin "foolysh::scene::Origin":
        TOP_LEFT,
        TOP_RIGHT,
        TOP_CENTER,
        CENTER_LEFT,
        CENTER,
        CENTER_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT,
        BOTTOM_CENTER

    cdef cppclass Size:
        Size() except +
        Size(const double, const double) except +
        double w
        double h

    cdef cppclass Scale:
        Scale() except +
        Scale(const double) except +
        Scale(const double, const double) except +
        double sx
        double sy
        Scale operator*(const Scale&)
        bint operator==(const double)
        bint operator!=(const double)

    cdef cppclass SceneGraphDataHandler:
        pass

    cdef cppclass Node:
        Node(SceneGraphDataHandler&) except +
        Node(const Node&) except +

        Node attach_node()
        void reparent_to(Node&) except +
        void reparent_to(const size_t) except +
        bint traverse(bint local = False) except +
        SmallList[size_t] query(AABB&, const bint) except +
        bint hidden()
        void hide()
        void show()
        void propagate_dirty()

        size_t get_id()
        size_t get_parent_id()
        void set_pos(const double)
        void set_pos(Node&, const double)
        void set_pos(const double, const double)
        void set_pos(Node&, const double, const double)
        void set_pos(Vec2&)
        void set_pos(Node&, Vec2&)
        Vec2 get_pos()
        Vec2 get_pos(Node&)
        void set_scale(const double)
        void set_scale(Node&, const double)
        void set_scale(const double, const double)
        void set_scale(Node&, const double, const double)
        void set_scale(const Scale&)
        void set_scale(Node&, const Scale&)
        Scale get_scale()
        Scale get_scale(Node&)
        void set_angle(double a, bint)
        void set_angle(Node&, double a, bint)
        double get_angle(bint)
        double get_angle(Node&, bint)
        void set_rotation_center(const double, const double)
        void set_rotation_center(Vec2&)
        Vec2 get_rotation_center()
        void set_depth(const int)
        void set_depth(Node&, const int)
        int get_depth()
        int get_depth(Node&)
        void set_origin(Origin)
        Origin get_origin()

        Vec2 get_relative_pos()
        Scale get_relative_scale()
        double get_relative_angle()
        int get_relative_depth()
        Size get_relative_size()

        void set_size(const Size&)
        void set_size(const double, const double)
        Size get_size()
        void set_distance_relative(const bint)
        bint get_distance_relative()

        AABB get_aabb()
