"""
Cython implementation of AABB class.
"""

from .vector cimport Vector
from .vector cimport Point

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


cdef class AABB:
    """
    Represents an Axis Aligned Bounding Box. Can be tested against
    with the overloaded operators (``<``, ``<=``, ``>=``, ``>``) as follows:

    >>> from engine.tools import vector
    >>>
    >>> a = AABB(box=(0.5, 0.5, 1.0, 1.0))
    >>> b = AABB(box=(0.5, 0.5, 0.7, 0.7))
    >>> a < b       # Is b completely inside a? (not touching)
    False
    >>> a <= b      # Is b inside a? (including exact overlap)
    True
    >>> a > b       # Is b overlapping a?
    True
    >>> a >= b      # Is b overlapping or touching a?
    True
    >>> a < vector.Point(0.75, 0.75)  # Is vector.Point at 0.75, 0.75 completely inside a?
    True

    :param box: ``4-Tuple[int/float]`` -> x1, y1, x2, y2 = top left and bottom
        right points of the bounding box.

    """
    def __init__(self, box, double precision=1e-6):
        if isinstance(box, tuple) and len(box) == 4 and \
                sum([isinstance(i, (int, float)) for i in box]) == 4:
            if box[0] < box[2] and box[1] < box[3]:
                self.x0, self.y0, self.x1, self.y1 = box
            else:
                raise ValueError(f'invalid bounding box specified: {str(box)}')
        else:
            raise TypeError(f'expected 4-Tuple[int/float], got type {type(box).__name__} with content {str(box)}')
        self._precision = precision

    @property
    def box(self):
        return self.x0, self.y0, self.x1, self.y1

    cdef bint _test_aabb(self, AABB other, int test_type):
        if test_type == 0:      # <=
            if self.x0 <= other.x0 and self.y0 <= other.y0 and self.x1 >= other.x1 \
                    and self.y1 >= other.y1:
                return True
        elif test_type == 1:    # <
            if self.x0 < other.x0 and self.y0 < other.y0 and self.x1 > other.x1 and self.y1 > other.y1:
                return True
        elif test_type == 2:    # >
            if self.x0 < other.x0 < self.x1 and self.y0 < other.y0 < self.y1:
                return True
            if self.x0 < other.x1 < self.x1 and self.y0 < other.y1 < self.y1:
                return True
            if other < self:
                return True
        elif test_type == 3:    # >=
            if self.x0 <= other.x0 <= self.x1 and self.y0 <= other.y0 <= self.y1:
                return True
            if self.x0 <= other.x1 <= self.x1 and self.y0 <= other.y1 <= self.y1:
                return True
            if other <= self:
                return True
        return False

    cdef bint _test_point(self, Point other, int test_type):
        lx = self.x0 - other.x
        hx = other.x - self.x1
        ly = self.y0 - other.y
        hy = other.y - self.y1
        if test_type == 0 or test_type == 3:
            if lx <= self._precision >= hx and ly <= self._precision >= hy:
                return True
        else:
            if lx < 0 > hx and ly < 0 > hy:
                return True
        return False

    cdef bint _test(self, other, int test_type):
        if isinstance(other, AABB):
            return self._test_aabb(other, test_type)
        elif isinstance(other, Point):
            return self._test_point(other, test_type)
        elif isinstance(other, Vector):
            return self._test_point(other.aspoint(), test_type)
        elif isinstance(other, (list, tuple)) and len(other) == 2:
            p = Point(tuple(other))
            return self._test_point(p, test_type)
        else:
            raise ValueError('Expected other to be of type AABB, vector.Point or '
                             'List/Tuple of length 2')

    cpdef bint inside(self, other, bint completely=False):
        """
        Test whether a box or point is inside this ``AABB``. This method yields
        the same results as using the ``<`` and ``<=`` operators would.

        :param other: ``AABB``, ``Vector/Point``, ``Iterable``
        :param completely: Optional ``bool`` -> whether ``other`` must lie
            entirely inside this ``AABB``.
        :return: ``bool``
        """
        if completely:
            return self._test(other, 0)
        return self._test(other, 1)

    cpdef bint overlap(self, other, bint touching=True):
        """
        Test whether a box or point overlaps with this ``AABB``. This method
        yields the same results as using the ``>=`` and ``>`` operators would.

        :param other: ``AABB``, ``Vector/Point``, ``Iterable``
        :param touching: Optional ``bool`` -> whether to include the border of
            this ``AABB``.
        :return: ``bool``
        """
        if touching:
            return self._test(other, 3)
        return self._test(other, 2)

    def __le__(self, other):
        return self._test(other, 0)

    def __lt__(self, other):
        return self._test(other, 1)

    def __gt__(self, other):
        return self._test(other, 2)

    def __ge__(self, other):
        return self._test(other, 3)

    def __len__(self):
        return 4

    def __contains__(self, item):
        return True if item in range(4) else False

    def __getitem__(self, item):
        if item in range(4):
            return self.box[item]
        raise IndexError

    def __repr__(self):
        return f'{type(self).__name__}{self.__str__()}'

    def __str__(self):
        return str(self.box)