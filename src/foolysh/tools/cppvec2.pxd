# distutils: language = c++
"""
Basic 2D Vector implementation.
"""

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

cdef extern from "src/vec2.cpp":
    pass

cdef extern from "src/vec2.hpp" namespace "foolysh::tools":
    cdef cppclass Vec2:
        Vec2() except +
        Vec2(const double) except +
        Vec2(const double, const double) except +
        Vec2(Vec2) except +

        double dot(const Vec2&)
        bint normalize()
        Vec2 normalized() except +
        double magnitude()
        double length()
        void rotate(double, bint)
        Vec2 rotated(double, bint)
        bint almost_equal(Vec2& other, const double)
        double& operator[](const int) except +
        Vec2 operator+(const Vec2&)
        Vec2 operator+(const double)
        Vec2 operator-(const Vec2&)
        Vec2 operator-(const double)
        Vec2 operator*(const double)
        Vec2 div(const double)
        void iadd(const Vec2&)
        void iadd(const double)
        void isub(const Vec2&)
        void isub(const double)
        void imul(const double)
        void idiv(const double)
        bint operator==(Vec2)
        bint operator!=(Vec2)
        bint operator==(const double)
        bint operator!=(const double)

