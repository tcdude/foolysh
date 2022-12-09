# distutils: language = c++
# cython: language_level=3
"""
Provides a simplistic Task Manager to execute either every frame or in a
specified interval.
"""

from .cpptaskmgr cimport TaskManager as _TaskManager
from .cpptaskmgr cimport callback
from .tools.cppclock cimport Clock

from cython.operator cimport dereference as deref

from libcpp.memory cimport unique_ptr
from libcpp.string cimport string
from libcpp cimport bool

import traceback
import warnings

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


cdef class Task:
    """
    Simulate a Task object for user convenience, providing direct access to a
    task, if the user wishes to store a reference.
    """
    cdef object _name
    cdef object _cb
    cdef object _args
    cdef object _kwargs
    cdef TaskManager _task_manager

    def __cinit__(self, name, cb, args, kwargs, task_manager):
        self._name = name
        self._cb = cb
        self._args = args
        self._kwargs = kwargs
        self._task_manager = task_manager

    cdef tuple get_callback(self):
        """Used to get a Tuple of callback, args and kwargs."""
        return self._cb, self._args, self._kwargs

    @property
    def delay(self):
        """
        ``float`` delay between execution of the Task.

        :setter: Resets the delay to the specified value.

        .. note::
            Setting :attr:`~foolysh.taskmanager.Task.delay` automatically resets
            remaining time until next execution to the specified value!
        """
        return self._task_manager.get_delay(self._name)

    @delay.setter
    def delay(self, value):
        if isinstance(value, (float, int)):
            if value < 0:
                raise ValueError('delay must be >= 0')
            self._task_manager.set_delay(self._name, <double> value)
        else:
            raise TypeError

    @property
    def name(self):
        """
        The identifier of this Task.
        """
        return self._name

    @property
    def ispaused(self):
        """
        ``bool``
        """
        return not self._task_manager.state(self._name)

    def pause(self):
        """Pauses the execution of :class:`~foolysh.taskmanager.Task`."""
        self._task_manager.pause(self._name)

    def resume(self):
        """Resumes the execution of :class:`~foolysh.taskmanager.Task`."""
        self._task_manager.resume(self._name)

    def __call__(self):
        """
        Manually execute the Task.
        """
        self._cb(*self._args, **self._kwargs)

    def __repr__(self):
        return f'{type(self).__name__}({self._name}, {self.delay:.4f})'

    def __str__(self):
        return self.__repr__()


cdef class TaskManager:
    """
    Provides a simple manager for regular execution of arbitrary user defined
    methods/functions.

    The method :meth:`~foolysh.taskmanager.TaskManager.add_task` returns a
    :class:`~foolysh.taskmanager.Task` object for convenience, but
    :class:`~foolysh.taskmanager.TaskManager` is also subscriptable, allowing
    for retrieving :class:`~foolysh.taskmanager.Task` objects by their ``name``:

    .. code-block:: python

        ...
        my_task = my_task_manager['name_of_my_task']  # retrieve the Task object
        my_task.pause()  # do something with the Task
        ...
    """
    cdef unique_ptr[_TaskManager] thisptr
    cdef dict _tasks
    cdef list _remove

    def __cinit__(self, *args, **kwargs):
        self.thisptr.reset(new _TaskManager())
        cdef callback _cb = <callback> self._run_callback
        deref(self.thisptr).set_callback(_cb)
        self._tasks = {}
        self._remove = []

    def __call__(self, dt):
        self.execute(dt)

    cdef void _run_callback(self, string name, const double dt,
                            const bool with_dt):
        """
        Proxy function to actually call the Python callback from a task.
        """
        cdef Task task
        task = self._tasks[name]
        cb, a, kw = task.get_callback()
        if with_dt:
            kw['dt'] = dt
        try:
            cb(*a, **kw)
        except Exception as err:
            warnings.warn(
                f'Error occurred while trying to execute task {name}: {err}\n'
                + traceback.format_exc())

    cpdef Task add_task(self, name, cb, double delay=0.0, bool with_dt=True,
                        args=None, kwargs=None):
        """
        Add a task to the :class:`TaskManager`.

        Args:
            name: ``str`` unique name of the task.
            cb: ``callable`` the callback method to be invoked by the task.
            delay: ``float`` optional delay of execution in seconds (default =
                ``0``).
            with_dt: whether the keyword argument ``dt`` should be added to the
                call to ``cb`` with the delta time since the last call (default
                = ``True``).
            args: ``tuple`` of optional positional arguments
            kwargs: ``dict`` of optional keyword arguments

        Returns:
            :class:`~foolysh.taskmanager.Task` instance of the added task.
        """
        if args is None:
            args = tuple()
        if kwargs is None:
            kwargs = dict()
        if not isinstance(name, bytes):
            name = name.encode('UTF-8')
        t = Task.__new__(Task, name, cb, args, kwargs, self)
        self._tasks[name] = t
        self._add_task(name, delay, with_dt)
        return t

    cdef _add_task(self, string name, double delay, bool with_dt):
        cdef void* pyobj
        pyobj = <void*> self
        deref(self.thisptr).add_task(name, delay, with_dt, pyobj)

    cpdef void remove_task(self, name):
        """
        Remove task created as ``name`` in :meth:`~TaskManager.add_task`.

        Args:
            name: ``str``
        """
        if not isinstance(name, bytes):
            name = name.encode('UTF-8')
        if name not in self._tasks:
            warnings.warn(f'No task named "{name}"', RuntimeWarning)
        deref(self.thisptr).remove_task(name)
        self._remove.append(name)

    cpdef void execute(self, const double dt):
        """
        Execute the TaskManager. This ticks the TaskManagers' clock forward and
        calls all scheduled/overdue tasks.
        """
        while len(self._remove) > 0:
            self._tasks.pop(self._remove.pop())
        deref(self.thisptr).execute(dt)

    cpdef void set_delay(self, name, const double delay):
        """
        Updates the delay of the task stored under ``name``.

        Args:
            name: ``str``
            delay: ``float`` new value of the delay.
        """
        if not isinstance(name, bytes):
            name = name.encode('UTF-8')
        if name not in self._tasks:
            raise ValueError(f'No task named "{name}"')
        deref(self.thisptr).set_delay(name, delay)

    cpdef void pause(self, name):
        """
        Pauses the task stored under ``name``.

        Args:
            name: ``str``
        """
        if not isinstance(name, bytes):
            name = name.encode('UTF-8')
        if name not in self._tasks:
            raise ValueError(f'No task named "{name}"')
        deref(self.thisptr).pause(name)

    cpdef void resume(self, name):
        """
        Resumes execution of the task stored under ``name``.

        Args:
            name: ``str``
        """
        if not isinstance(name, bytes):
            name = name.encode('UTF-8')
        if name not in self._tasks:
            raise ValueError(f'No task named "{name}"')
        deref(self.thisptr).resume(name)

    cpdef bool state(self, name):
        """
        Query the state of the task stored under ``name``.

        Args:
            name: ``str``

        Returns:
            ``bool`` ``True`` if the task is running, otherwise ``False``.
        """
        if not isinstance(name, bytes):
            name = name.encode('UTF-8')
        if name not in self._tasks:
            raise ValueError(f'No task named "{name}"')
        return deref(self.thisptr).state(name)

    cpdef double get_delay(self, name):
        """
        Get the current delay for task stored under ``name``.

        Args:
            name: ``str``

        Returns:
            ``float`` delay in seconds.
        """
        name = name.encode('UTF-8')
        if name not in self._tasks:
            raise ValueError(f'No task named "{name}"')
        return deref(self.thisptr).get_delay(name)

    def __getitem__(self, item):
        if not isinstance(item, bytes):
            item = item.encode('UTF-8')
        if item in self._tasks:
            return self._tasks[item]
        raise IndexError

    def __repr__(self):
        return f'{type(self).__name__}({len(self._tasks)})'

    def __str__(self):
        return self.__repr__()
