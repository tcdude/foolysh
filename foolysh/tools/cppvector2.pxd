# distutils: language = c++
"""
Basic 2D Vector implementation.
"""

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


cdef extern from "src/common.hpp":
    pass

cdef extern from "src/vector2.cpp":
    pass

cdef extern from "src/vector2.hpp" namespace "tools":
    cdef cppclass Vector2:
        Vector2() except +
        Vector2(const double) except +
        Vector2(const double, const double) except +
        Vector2(Vector2) except +

        double dot(const Vector2&)
        bint normalize()
        Vector2 normalized() except +
        double magnitude()
        double length()
        void rotate(double, bint)
        Vector2 rotated(double, bint)
        bint almost_equal(Vector2& other, const double)
        double& operator[](const int) except +
        Vector2 operator+(const Vector2&)
        Vector2 operator+(const double)
        Vector2 operator-(const Vector2&)
        Vector2 operator-(const double)
        Vector2 operator*(const double)
        Vector2 div(const double)
        void iadd(const Vector2&)
        void iadd(const double)
        void isub(const Vector2&)
        void isub(const double)
        void imul(const double)
        void idiv(const double)
        bint operator==(Vector2)
        bint operator!=(Vector2)
        bint operator==(const double)
        bint operator!=(const double)

