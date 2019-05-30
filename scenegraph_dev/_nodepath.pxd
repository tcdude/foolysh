# distutils: language = c++
"""
NodePath implementation.
"""

from _vector2 cimport Vector2
from _aabb cimport AABB

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

cdef extern from "src/quadtree.cpp":
    pass

cdef extern from "src/quadtree.hpp":
    pass

cdef extern from "src/list_t.cpp":
    pass

cdef extern from "src/list_t.hpp" namespace "":
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

cdef extern from "src/nodepath.cpp":
    pass

cdef extern from "src/nodepath.hpp" namespace "scenegraph":
    cdef enum Origin "scenegraph::Origin":
        TOP_LEFT,
        TOP_RIGHT,
        TOP_CENTER,
        CENTER_LEFT,
        CENTER,
        CENTER_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT,
        BOTTOM_CENTER
    
    cdef cppclass Size "scenegraph::Size":
        double w
        double h

    cdef cppclass Scale "scenegraph::Scale":
        double sx
        double sy
        Scale operator*(const Scale&)
        bint operator==(const double)
        bint operator!=(const double)

    cdef cppclass ChildNodePath "scenegraph::ChildNodePath":
        int node_path_id
        int next

    cdef cppclass QuadtreeEntry "scenegraph::QuadtreeEntry":
        int node_path_id
        AABB aabb

    cdef cppclass NodePath:
        NodePath() except +
        NodePath(const NodePath&) except +

        NodePath attach_new_node_path()
        void reparent_to(NodePath&)
        bint traverse() except +
        SmallList[int] query(AABB&)
        void hide()
        void show()
        
        int get_id()
        void set_pos(const double)
        void set_pos(const double, const double)
        void set_pos(Vector2&)
        Vector2 get_pos()
        void set_scale(const double)
        void set_scale(const double, const double)
        void set_scale(const Scale&)
        Scale get_scale()
        void set_angle(double a, bint)
        double get_angle(bint)
        void set_rotation_center(const double, const double)
        void set_rotation_center(Vector2&)
        Vector2 get_rotation_center()
        void set_depth(const int)
        int get_depth()
        void set_origin(Origin)
        Origin get_origin()

        Vector2 get_relative_pos()
        Scale get_relative_scale()
        double get_relative_angle()
        int get_relative_depth()

        void set_size(const Size&)
        void set_size(const double, const double)
        Size get_size()
        void set_distance_relative(const bint)
        bint get_distance_relative()
        
        @staticmethod
        NodePath& get_node_path(const int)
