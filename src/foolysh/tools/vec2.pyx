# distutils: language = c++
"""
Basic 2D Vector implementation.
"""

from .cppvec2 cimport Vec2 as _Vec2

cimport cython
from cython.operator cimport dereference as deref

from libcpp.memory cimport unique_ptr

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


@cython.freelist(10)
cdef class Vec2:
    """
    Basic 2D Vector implementation with the following overloaded operators:

      * Arithmetic operators ``(+, +=, -, -=, *, *=, /, /=)``
      * Comparison against other ``Vec2`` objects and scalars ``(==, !=)``
    """
    def __cinit__(self, *args, **kwargs):
        if args:
            if len(args) == 1:
                if isinstance(args[0], (int, float)):
                    self.thisptr.reset(new _Vec2(<double> args[0]))
                elif isinstance(args[0], tuple) and len(args[0]) == 2 \
                     and isinstance(args[0][0], (int, float)) \
                     and isinstance(args[0][1], (int, float)):
                    self.thisptr.reset(
                        new _Vec2(<double> args[0][0], <double> args[0][1])
                    )
                else:
                    raise TypeError
            elif len(args) == 2 and isinstance(args[0], (int, float)) \
                 and isinstance(args[1], (int, float)):
                self.thisptr.reset(new _Vec2(args[0], args[1]))
        else:
            self.thisptr.reset(new _Vec2())

    def dot(self, other):
        """
        Args:
            other: ``Vec2``

        Returns:
            Dot product of this and `other`.
        """
        if not isinstance(other, Vec2):
            raise TypeError(f'Expected type Vec2, got '
                            f'{type(other).__name__} instead.')
        return self._dot(other)

    cdef double _dot(self, Vec2 other):
        return deref(self.thisptr).dot(deref(other.thisptr))

    def normalize(self):
        """
        Normalize the vector to unit length.

        Returns:
            ``True`` if successful, otherwise ``False``.
        """
        return deref(self.thisptr).normalize()

    def normalized(self):
        """
        Returns:
            ``Vec2`` of this vector normalized to unit length.
        """
        return self._normalized()

    cdef Vec2 _normalized(self) except +:
        cdef _Vec2* v = new _Vec2(
            deref(self.thisptr)[0], deref(self.thisptr)[1]
        )
        v.normalize()
        ret = Vec2.__new__(Vec2, deref(v)[0], deref(v)[1])
        del v
        return ret

    @property
    def magnitude(self):
        """
        Returns:
            ``float`` the squared length of this vector.
        """
        return deref(self.thisptr).magnitude()

    @property
    def length(self):
        """
        Returns:
            ``float`` the length of this vector.
        """
        return deref(self.thisptr).length()

    cpdef void rotate(self, double a, bint radians=False):
        """
        Rotates this vector around the origin.

        Args:
            a: ``float`` angle in degrees or radians
            radians: ``bool`` whether to use radians or degrees
                (default = ``False``)
        """
        deref(self.thisptr).rotate(a, radians)

    cpdef Vec2 rotated(self, double a, bint radians=False):
        """

        Args:
            a: ``float`` angle in degrees or radians
            radians: ``bool`` whether to use radians or degrees

        Returns:
            ``Vec2`` of the rotated vector around the origin.
        """
        cdef _Vec2 v = deref(self.thisptr).rotated(a, radians)
        return Vec2(v[0], v[1])

    @property
    def x(self):
        """
        ``int`` / ``float`` x component of the vector
        """
        return deref(self.thisptr)[0]

    @x.setter
    def x(self, value):
        if isinstance(value, (int, float)):
            deref(self.thisptr)[0] = value
        else:
            raise TypeError

    @property
    def y(self):
        """
        ``int`` / ``float`` y component of the vector
        """
        return deref(self.thisptr)[1]

    @y.setter
    def y(self, value):
        if isinstance(value, (int, float)):
            deref(self.thisptr)[1] = value
        else:
            raise TypeError

    def __add__(self, other):
        if isinstance(other, Vec2) and isinstance(self, Vec2):
            return self._add(other)
        elif isinstance(other, (int, float)) and isinstance(self, Vec2):
            return self._add_scalar(other)
        elif isinstance(self, (int, float)) and isinstance(other, Vec2):
            return other._add_scalar(self)
        return NotImplemented

    def __iadd__(self, other):
        if self._iadd(other):
            return self
        return NotImplemented

    cpdef Vec2 _add(self, Vec2 other):
        cdef _Vec2* v = new _Vec2(
            deref(self.thisptr) + deref(other.thisptr)
        )
        try:
            return Vec2.__new__(Vec2, deref(v)[0], deref(v)[1])
        finally:
            del v

    def _iadd(self, other):
        if isinstance(other, Vec2):
            self._iadd_vec(other)
            return True
        elif isinstance(other, (int, float)):
            deref(self.thisptr).iadd(<double> other)
            return True
        return False

    cdef void _iadd_vec(Vec2 self, Vec2 other):
        deref(self.thisptr).iadd(<_Vec2> deref(other.thisptr))

    cpdef Vec2 _add_scalar(self, const double other):
        cdef _Vec2* v = new _Vec2(deref(self.thisptr) + other)
        try:
            return Vec2.__new__(Vec2, deref(v)[0], deref(v)[1])
        finally:
            del v

    def __sub__(self, other):
        if isinstance(other, Vec2) and isinstance(self, Vec2):
            return self._sub(other)
        elif isinstance(other, (int, float)) and isinstance(self, Vec2):
            return self._sub_scalar(other)
        elif isinstance(self, (int, float)) and isinstance(other, Vec2):
            return other._sub_scalar_r(self)
        return NotImplemented

    def __isub__(self, other):
        if self._isub(other):
            return self
        return NotImplemented

    cpdef Vec2 _sub(self, Vec2 other):
        cdef _Vec2* v = new _Vec2(
            deref(self.thisptr) - deref(other.thisptr)
        )
        try:
            return Vec2.__new__(Vec2, deref(v)[0], deref(v)[1])
        finally:
            del v

    def _isub(self, other):
        if isinstance(other, Vec2):
            self._iadd_vec(other)
            return True
        elif isinstance(other, (int, float)):
            deref(self.thisptr).isub(<double> other)
            return True
        return False
    cdef void _isub_vec(Vec2 self, Vec2 other):
        deref(self.thisptr).isub(<_Vec2> deref(other.thisptr))

    cpdef Vec2 _sub_scalar(self, const double other):
        cdef _Vec2* v = new _Vec2(deref(self.thisptr) - other)
        try:
            return Vec2.__new__(Vec2, deref(v)[0], deref(v)[1])
        finally:
            del v

    cpdef Vec2 _sub_scalar_r(self, const double other):
        cdef _Vec2* v = new _Vec2(
            other - deref(self.thisptr)[0], other - deref(self.thisptr)[1]
        )
        try:
            return Vec2.__new__(Vec2, deref(v)[0], deref(v)[1])
        finally:
            del v

    def __neg__(self):
        return self._neg()

    cpdef Vec2 _neg(self):
        return Vec2.__new__(Vec2, -deref(self.thisptr)[0], -deref(self.thisptr)[1])

    def __mul__(self, other):
        if isinstance(other, (int, float)) and isinstance(self, Vec2):
            return self._mul(other)
        elif isinstance(self, (int, float)) and isinstance(other, Vec2):
            return other._mul(self)
        return NotImplemented

    def __imul__(self, other):
        if self._imul(other):
            return self
        return NotImplemented

    cpdef Vec2 _mul(self, const double other):
        cdef _Vec2* v = new _Vec2(deref(self.thisptr) * other)
        try:
            return Vec2.__new__(Vec2, deref(v)[0], deref(v)[1])
        finally:
            del v

    def _imul(self, other):
        if isinstance(other, (int, float)):
            deref(self.thisptr).imul(<double> other)
            return True
        return False

    def __truediv__(self, other):
        if isinstance(other, (int, float)) and isinstance(self, Vec2):
            return self._tdiv(other)
        elif isinstance(self, (int, float)) and isinstance(other, Vec2):
            return other._tdiv(self)
        return NotImplemented

    def __itruediv__(self, other):
        if self._idiv(other):
            return self._tdiv(other)
        return NotImplemented

    def _idiv(self, other):
        if isinstance(other, (int, float)):
            deref(self.thisptr).idiv(other)
            return True
        return False

    cpdef Vec2 _tdiv(self, double value):
        cdef _Vec2* v = new _Vec2(deref(self.thisptr).div(<double> value))
        try:
            return Vec2.__new__(Vec2, deref(v)[0], deref(v)[1])
        finally:
            del v

    def __eq__(self, other):
        if isinstance(self, Vec2) and isinstance(other, Vec2):
            return self._eq(other)
        elif isinstance(self, Vec2) and isinstance(other, (int, float)):
            return self._eq_scalar(other)
        elif isinstance(other, Vec2) and isinstance(self, (int, float)):
            return other._eq_scalar(self)
        return NotImplemented

    cpdef bint _eq(self, Vec2 other):
        return deref(self.thisptr) == deref(other.thisptr)

    cpdef bint _eq_scalar(self, double other):
        return deref(self.thisptr) == other

    def __ne__(self, other):
        if isinstance(self, Vec2) and isinstance(other, Vec2):
            return self._ne(other)
        elif isinstance(self, Vec2) and isinstance(other, (int, float)):
            return self._ne_scalar(other)
        elif isinstance(other, Vec2) and isinstance(self, (int, float)):
            return other._ne_scalar(self)
        return NotImplemented

    cpdef bint _ne(self, Vec2 other):
        return deref(self.thisptr) != deref(other.thisptr)

    cpdef bint _ne_scalar(self, double other):
        return deref(self.thisptr) != other

    cdef _Vec2 vec2(self):
        return deref(self.thisptr)

    def __getitem__(self, key):
        if key in (0, 'x'):
            return self.x
        if key in (1, 'y'):
            return self.y
        raise IndexError(f'Invalid Index "{key}" for Vec2 object')

    def __setitem__(self, key, value):
        if key in (0, 'x'):
            self.x = value
        elif key in (1, 'y'):
            self.y = value
        else:
            raise IndexError(f'Invalid Index "{key}" for Vec2 object')

    def __len__(self):
        # type: () -> int
        return 2

    def __repr__(self):
        return f'{type(self).__name__}{str(self)}'

    def __str__(self):
        cdef double x, y
        x = deref(self.thisptr)[0]
        y = deref(self.thisptr)[1]
        return f'({x:.4}, {y:.4})'


cdef class Point2(Vec2):
    """
    Stub class to represent a Point2. Underlying there is no difference between
    this and :class:`Vec2`, other than the fact that ``repr`` will represent
    this as ``Point2``.
    """
    pass
