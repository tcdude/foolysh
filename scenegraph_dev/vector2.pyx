# distutils: language = c++
"""
Basic 2D Vector implementation.
"""

from _vector2 cimport Vector2 as _Vector2

cimport cython
from cython.operator cimport dereference as deref

from libcpp.memory cimport unique_ptr

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


@cython.freelist(10)
cdef class Vector2:
    def __cinit__(self, *args, **kwargs):
        if args:
            if len(args) == 1:
                if isinstance(args[0], (int, float)):
                    self.thisptr.reset(new _Vector2(<double> args[0]))
                else:
                    raise TypeError
            elif len(args) == 2 and isinstance(args[0], (int, float)) \
                 and isinstance(args[1], (int, float)):
                self.thisptr.reset(new _Vector2(args[0], args[1]))

    def dot(self, other):
        if not isinstance(other, Vector2):
            raise TypeError(f'Expected type Vector2, got '
                            f'{type(other).__name__} instead.')
        return self._dot(other)

    cdef double _dot(self, Vector2 other):
        return deref(self.thisptr).dot(deref(other.thisptr))

    def normalize(self):
        return deref(self.thisptr).normalize()

    def normalized(self):
        return self._normalized()

    cdef Vector2 _normalized(self):
        cdef _Vector2* v = new _Vector2(deref(self.thisptr)[0], deref(self.thisptr)[1])
        v.normalize()
        ret = Vector2.__new__(Vector2, deref(v)[0], deref(v)[1])
        del v
        return ret

    @property
    def magnitude(self):
        return deref(self.thisptr).magnitude()

    @property
    def length(self):
        return deref(self.thisptr).length()

    cpdef void rotate(self, double a, bint radians=False):
        deref(self.thisptr).rotate(a, radians)

    cpdef Vector2 rotated(self, double a, bint radians=False):
        cdef _Vector2 v = deref(self.thisptr).rotated(a, radians)
        return Vector2(v[0], v[1])

    @property
    def x(self):
        return deref(self.thisptr)[0]

    @x.setter
    def x(self, value):
        if isinstance(value, (int, float)):
            deref(self.thisptr)[0] = value
        else:
            raise TypeError

    @property
    def y(self):
        return deref(self.thisptr)[1]

    @y.setter
    def y(self, value):
        if isinstance(value, (int, float)):
            deref(self.thisptr)[1] = value
        else:
            raise TypeError

    def __add__(self, other):
        if isinstance(other, Vector2) and isinstance(self, Vector2):
            return self.add(other)
        elif isinstance(other, (int, float)) and isinstance(self, Vector2):
            return self.add_scalar(other)
        elif isinstance(self, (int, float)) and isinstance(other, Vector2):
            return other.add_scalar(self)
        return NotImplemented

    def __iadd__(self, other):
        if self.iadd(other):
            return self
        return NotImplemented

    cpdef Vector2 add(self, Vector2 other):
        cdef _Vector2* v = new _Vector2(deref(self.thisptr) + deref(other.thisptr))
        try:
            return Vector2.__new__(Vector2, deref(v)[0], deref(v)[1])
        finally:
            del v

    def iadd(self, other):
        if isinstance(other, Vector2):
            self._iadd_vec(other)
            return True
        elif isinstance(other, (int, float)):
            deref(self.thisptr).iadd(<double> other)
            return True
        return False

    cdef void _iadd_vec(Vector2 self, Vector2 other):
        deref(self.thisptr).iadd(<_Vector2> deref(other.thisptr))

    cpdef Vector2 add_scalar(self, const double other):
        cdef _Vector2* v = new _Vector2(deref(self.thisptr) + other)
        try:
            return Vector2.__new__(Vector2, deref(v)[0], deref(v)[1])
        finally:
            del v

    def __sub__(self, other):
        if isinstance(other, Vector2) and isinstance(self, Vector2):
            return self.sub(other)
        elif isinstance(other, (int, float)) and isinstance(self, Vector2):
            return self.sub_scalar(other)
        elif isinstance(self, (int, float)) and isinstance(other, Vector2):
            return other.sub_scalar_r(self)
        return NotImplemented

    def __isub__(self, other):
        if self.isub(other):
            return self
        return NotImplemented

    cpdef Vector2 sub(self, Vector2 other):
        cdef _Vector2* v = new _Vector2(deref(self.thisptr) - deref(other.thisptr))
        try:
            return Vector2.__new__(Vector2, deref(v)[0], deref(v)[1])
        finally:
            del v

    def isub(self, other):
        if isinstance(other, Vector2):
            self._iadd_vec(other)
            return True
        elif isinstance(other, (int, float)):
            deref(self.thisptr).isub(<double> other)
            return True
        return False
    cdef void _isub_vec(Vector2 self, Vector2 other):
        deref(self.thisptr).isub(<_Vector2> deref(other.thisptr))

    cpdef Vector2 sub_scalar(self, const double other):
        cdef _Vector2* v = new _Vector2(deref(self.thisptr) - other)
        try:
            return Vector2.__new__(Vector2, deref(v)[0], deref(v)[1])
        finally:
            del v

    cpdef Vector2 sub_scalar_r(self, const double other):
        cdef _Vector2* v = new _Vector2(other - deref(self.thisptr)[0], other - deref(self.thisptr)[1])
        try:
            return Vector2.__new__(Vector2, deref(v)[0], deref(v)[1])
        finally:
            del v

    def __neg__(self):
        return self.neg()

    cpdef Vector2 neg(self):
        return Vector2.__new__(Vector2, -deref(self.thisptr)[0], -deref(self.thisptr)[1])

    def __mul__(self, other):
        if isinstance(other, (int, float)) and isinstance(self, Vector2):
            return self.mul(other)
        elif isinstance(self, (int, float)) and isinstance(other, Vector2):
            return other.mul(self)
        return NotImplemented

    def __imul__(self, other):
        if self.imul(other):
            return self
        return NotImplemented

    cpdef Vector2 mul(self, const double other):
        cdef _Vector2* v = new _Vector2(deref(self.thisptr) * other)
        try:
            return Vector2.__new__(Vector2, deref(v)[0], deref(v)[1])
        finally:
            del v

    def imul(self, other):
        if isinstance(other, (int, float)):
            deref(self.thisptr).imul(<double> other)
            return True
        return False

    def __truediv__(self, other):
        if isinstance(other, (int, float)) and isinstance(self, Vector2):
            return self.tdiv(other)
        elif isinstance(self, (int, float)) and isinstance(other, Vector2):
            return other.tdiv(self)
        return NotImplemented

    def __itruediv__(self, other):
        if self.idiv(other):
            return self.tdiv(other)
        return NotImplemented

    def idiv(self, other):
        if isinstance(other, (int, float)):
            deref(self.thisptr).idiv(other)
            return True
        return False

    cpdef Vector2 tdiv(self, double value):
        cdef _Vector2* v = new _Vector2(deref(self.thisptr).div(<double> value))
        try:
            return Vector2.__new__(Vector2, deref(v)[0], deref(v)[1])
        finally:
            del v

    def __eq__(self, other):
        if isinstance(self, Vector2) and isinstance(other, Vector2):
            return self.eq(other)
        elif isinstance(self, Vector2) and isinstance(other, (int, float)):
            return self.eq_scalar(other)
        elif isinstance(other, Vector2) and isinstance(self, (int, float)):
            return other.eq_scalar(self)
        return NotImplemented

    cpdef bint eq(self, Vector2 other):
        return deref(self.thisptr) == deref(other.thisptr)

    cpdef bint eq_scalar(self, double other):
        return deref(self.thisptr) == other

    def __ne__(self, other):
        if isinstance(self, Vector2) and isinstance(other, Vector2):
            return self.ne(other)
        elif isinstance(self, Vector2) and isinstance(other, (int, float)):
            return self.ne_scalar(other)
        elif isinstance(other, Vector2) and isinstance(self, (int, float)):
            return other.ne_scalar(self)
        return NotImplemented

    cpdef bint ne(self, Vector2 other):
        return deref(self.thisptr) != deref(other.thisptr)

    cpdef bint ne_scalar(self, double other):
        return deref(self.thisptr) != other

    cdef _Vector2 vector2(self):
        return deref(self.thisptr)

    def __repr__(self):
        return f'{type(self).__name__}{str(self)}'

    def __str__(self):
        cdef double x, y
        x = deref(self.thisptr)[0]
        y = deref(self.thisptr)[1]
        return f'({x:.4}, {y:.4})'
