# distutils: language = c++
"""
Provides Cython header for the Animation System.
"""

from libcpp cimport bool
from libcpp.memory cimport unique_ptr
from .scene.cppnode cimport Node
from .scene.cppnode cimport Scale
from .tools.cppvec2 cimport Vec2

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


cdef extern from "src/animation.cpp" namespace "foolysh::animation":
    pass

cdef extern from "src/animation.hpp" namespace "foolysh::animation":
    cdef enum BlendType "foolysh::animation::BlendType":
        NO_BLEND,
        EASE_IN,
        EASE_OUT,
        EASE_IN_OUT

    cdef cppclass AnimationBase:
        void loop(const bool)

    cdef cppclass AnimationType(AnimationBase):
        AnimationType() except +

        void set_node(Node)
        void set_blend(BlendType)

    cdef cppclass Interval(AnimationType):
        void set_duration(const double)

        void add_pos(Vec2)
        void add_pos(Vec2, Node)
        void add_pos(Vec2, Vec2)
        void add_pos(Vec2, Vec2, Node)

        void add_scale(Scale)
        void add_scale(Scale, Node)
        void add_scale(Scale, Scale)
        void add_scale(Scale, Scale, Node)

        void add_rotation(double)
        void add_rotation(double, Node)
        void add_rotation(double, double)
        void add_rotation(double, double, Node)

        void add_rotation_center(Vec2)
        void add_rotation_center(Vec2, Vec2)

        void add_depth(int)
        void add_depth(int, Node)
        void add_depth(int, int)
        void add_depth(int, int, Node)

    cdef cppclass Animation(AnimationType):
        void set_pos_speed(const double)
        void set_scale_speed(const double)
        void set_rotation_speed(const double)
        void set_rotation_center_speed(const double)
        void set_depth_speed(const double)

        void add_pos(Vec2)
        void add_pos(Vec2, Node)
        void add_pos(Vec2, Vec2)
        void add_pos(Vec2, Vec2, Node)

        void add_scale(Scale)
        void add_scale(Scale, Node)
        void add_scale(Scale, Scale)
        void add_scale(Scale, Scale, Node)

        void add_rotation(double)
        void add_rotation(double, Node)
        void add_rotation(double, double)
        void add_rotation(double, double, Node)

        void add_rotation_center(Vec2)
        void add_rotation_center(Vec2, Vec2)

        void add_depth(int)
        void add_depth(int, Node)
        void add_depth(int, int)
        void add_depth(int, int, Node)

    cdef cppclass Sequence(AnimationBase):
        pass

    cdef cppclass AnimationManager:
        int new_interval()
        int new_animation()
        int new_sequence()
        Interval& get_interval(const int) except +
        Animation& get_animation(const int) except +
        Sequence& get_sequence(const int) except +
        void remove_interval(const int) except +
        void remove_animation(const int) except +
        void remove_sequence(const int) except +
        void play_interval(const int) except +
        void play_animation(const int) except +
        void play_sequence(const int) except +
        void pause_interval(const int) except +
        void pause_animation(const int) except +
        void pause_sequence(const int) except +
        void resume_interval(const int) except +
        void resume_animation(const int) except +
        void resume_sequence(const int) except +
        void stop_interval(const int) except +
        void stop_animation(const int) except +
        void stop_sequence(const int) except +
        char get_interval_status(const int) except +
        char get_animation_status(const int) except +
        char get_sequence_status(const int) except +
        void append(const int, const int)

        void animate(const double)
