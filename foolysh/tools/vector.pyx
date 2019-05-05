"""
Cython implementation of Vector and Point classes.
"""

from libc.math cimport sin
from libc.math cimport cos
from libc.math cimport fabs
from libc.math cimport sqrt
from libc.math cimport round
from libc.math cimport pi


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


cdef inline double radians(double degrees):
    return (degrees / 180.0) * pi


cdef class Vector:
    def __init__(self, x=0, y=0, precision=1e-6):
        if isinstance(x, tuple):
            if len(x) == 2:
                x, y = x
            else:
                raise TypeError('expected tuple of length 2')
        if not isinstance(x, (int, float)) or not isinstance(y, (int, float)):
            raise TypeError('numerical types expected for x and y')
        self._x = x
        self._y = y
        self._precision = precision
        self._length = 0.0
        self._dirty = 1
        self._rtype = Vector

    def __repr__(self):
        return f'{type(self).__name__}{str(self)}'

    def __str__(self):
        return str((self._x, self._y))

    @property
    def x(self):
        """``x`` value"""
        return self._x

    @property
    def y(self):
        """``y`` value"""
        return self._y

    @x.setter
    def x(self, v):
        if isinstance(v, (int, float)):
            self._x = v
            self._dirty = 1
        else:
            raise TypeError('Must be of type int or float')

    @y.setter
    def y(self, v):
        if isinstance(v, (int, float)):
            self._y = v
            self._dirty = 1
        else:
            raise TypeError('Must be of type int or float')

    @property
    def rtype(self):
        return self._rtype

    @property
    def length(self):
        """``length`` of the vector"""
        if self._dirty:
            self._length = sqrt(self._x ** 2 + self._y ** 2)
            self._dirty = 0
        return self._length

    cpdef Vector normalized(self):
        """Returns a normalized Vector of this Vector instance."""
        cdef double vlen = self.length
        if vlen:
            return Vector(self._x / vlen, self._y / vlen)
        raise ValueError('Vector of zero length cannot be normalized')

    cpdef normalize(self):
        """
        Normalizes the Vector and returns True if successful. Raises a
        ValueError if the Vector is of zero length.
        """
        vlen = self.length
        if vlen:
            self._dirty = 1
            self._x /= vlen
            self._y /= vlen
            return True
        raise ValueError('Vector of zero length cannot be normalized')

    cpdef Vector rotate(self, degrees):
        """
        Returns a Vector rotated ``degrees`` around the origin.

        :param degrees: int/float -> angle of rotation in degrees.
        """
        a = radians(-degrees)
        sa = sin(a)
        ca = cos(a)
        return Vector(ca * self._x - sa * self._y, sa * self._x + ca * self._y)

    cpdef dot(self, Vector other):
        """
        Returns the dot product of ``this`` ⋅ ``other``

        :param other: Vector -> the other Vector for the dot product.
        """
        if isinstance(other, Vector):
            return self._x * other._x + self._y * other._y

    cpdef Vector asint(self, bint rounding=False):
        """
        Returns the Vector with its values cast to int. If rounding is True,
        rounds the value first, before casting (default=False).

        :param rounding: Optional bool -> Whether to round the values first.
        """
        if rounding:
            return self._rtype(<int> round(self._x), <int> round(self._y))
        return self._rtype(<int> self._x, <int> self._y)

    cpdef Point aspoint(self):
        """Returns the current Vector as Point."""
        return Point(self._x, self._y)

    def almost_equal(self, other, d=1e-6):
        if isinstance(other, Vector):
            return self._almost_equal(other, d)
        raise TypeError('expected Vector type')

    cdef bint _almost_equal(self, Vector other, double d=1e-6):
        """Returns ``True`` if difference is less than or equal to ``d``."""
        return fabs(self._x - other._x) <= d and fabs(self._y - other._y) <= d

    def __getitem__(self, key):
        if key in (0, 'x'):
            return self._x
        if key in (1, 'y'):
            return self._y
        raise IndexError('Invalid Index for Vector2 object')

    def __len__(self):
        return 2

    def __add__(self, other):
        if isinstance(self, Vector) and isinstance(other, (int, float)):
            return self._rtype(self._x + other, self._y + other)
        elif isinstance(self, Vector) and isinstance(other, Vector):
            return self._rtype(self._x + other._x, self._y + other._y)
        elif isinstance(other, Vector) and isinstance(self, (int, float)):
            return other._rtype(other._x + self, other._y + self)
        elif isinstance(other, tuple) and len(other) == 2 and \
                isinstance(other[0], (int, float)) and \
                isinstance(other[1], (int, float)) and isinstance(self, Vector):
            return self._rtype(self._x + other[0], self._y + other[1])
        elif isinstance(self, tuple) and len(self) == 2 and \
                isinstance(self[0], (int, float)) and \
                isinstance(self[1], (int, float)) and isinstance(other, Vector):
            return other._rtype(other._x + self[0], other._y + self[1])
        else:
            raise TypeError('Must be of type Vector, tuple, int or float')

    def __sub__(self, other):
        if isinstance(self, Vector) and isinstance(other, Vector):
            return self._rtype(self._x - other._x, self._y - other._y)
        elif isinstance(self, Vector) and isinstance(other, (int, float)):
            return self._rtype(self._x - other, self._y - other)
        elif isinstance(other, Vector) and isinstance(self, (int, float)):
            return other._rtype(self - other._x, self - other._y)
        elif isinstance(other, tuple) and len(other) == 2 and \
                isinstance(other[0], (int, float)) and \
                isinstance(other[1], (int, float)) and isinstance(self, Vector):
            return self._rtype(self._x - other[0], self._y - other[1])
        elif isinstance(self, tuple) and len(self) == 2 and \
                isinstance(self[0], (int, float)) and \
                isinstance(self[1], (int, float)) and isinstance(other, Vector):
            return other._rtype(self[0] - other._x, self[1] - other._y)
        else:
            raise TypeError('Must be of type Vector, tuple, int or float')

    def __mul__(self, other):
        if isinstance(self, Vector) and isinstance(other, (int, float)):
            return self._rtype(self._x * other, self._y * other)
        elif isinstance(self, (int, float)) and isinstance(other, Vector):
            return other._rtype(self * other._x, self * other._y)
        else:
            raise TypeError('Must be of type int or float')

    def __truediv__(self, other):
        if isinstance(self, Vector) and isinstance(other, (int, float)):
            return self._rtype(self._x / other, self._y / other)
        elif isinstance(self, (int, float)) and isinstance(other, Vector):
            return other._rtype(self / other._x, self / other._y)
        else:
            raise TypeError('Must be of type int or float')

    def __floordiv__(self, other):
        if isinstance(self, Vector) and isinstance(other, (int, float)):
            return self._rtype(self._x // other, self._y // other)
        elif isinstance(self, (int, float)) and isinstance(other, Vector):
            return other._rtype(self // other._x, self // other._y)
        else:
            raise TypeError('Must be of type int or float')

    def __neg__(self):
        return self._rtype(-self._x, -self._y)

    def __eq__(self, other):
        if not isinstance(other, Vector):
            return False
        return self._almost_equal(other, self._precision)


cdef class Point(Vector):
    def __init__(self, x=0, y=0):
        Vector.__init__(self, x, y)
        self._rtype = Point
