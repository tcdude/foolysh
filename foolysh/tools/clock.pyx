# distutils: language = c++
"""
Simple clock measuring at the highest resolution possible.
"""

from .cppclock cimport Clock as _Clock

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


cdef class Clock:
    """
    Provides accurate timing and methods to retrieve delta time and time passed
    since the first measurement took place.

    .. note::
        A :class:`Clock` instance is available in :class:`~foolysh.app.App` and
        only the methods :meth:`~Clock.get_dt` and :meth:`~Clock.get_time`
        should be used by the user.
    """
    cdef unique_ptr[_Clock] thisptr

    def __cinit__(self, *args, **kwargs):
        self.thisptr.reset(new _Clock())

    cpdef double get_dt(self):
        """
        Returns:
            ``float`` delta time in seconds between the last two calls to
            :meth:`~Clock.tick`.
        """
        return deref(self.thisptr).get_dt()

    cpdef double get_time(self):
        """
        Returns:
            ``float`` time in seconds since the first call to
            :meth:`~Clock.tick`.
        """
        return deref(self.thisptr).get_time()

    cpdef void tick(self):
        """
        When calling this, measures time and updates current and delta time.

        .. note::
            Use this to measure steps, if you're using a separate :class:`Clock`
            instance from the one provided in :class:`~foolysh.app.App`, where
            this method has been overridden to prevent direct access.
        """
        deref(self.thisptr).tick()
