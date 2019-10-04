# distutils: language = c++
"""
Node implementation.
"""

from enum import Enum

from .cppnode cimport Node as _Node
from .cppnode cimport SmallList
from .cppnode cimport Scale
from .cppnode cimport Size
from .cppnode cimport Origin as _Origin
from ..tools.cppaabb cimport AABB as _AABB
from ..tools.aabb cimport AABB
from ..tools.cppvector2 cimport Vector2 as _Vector2
from ..tools.vector2 cimport Vector2

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

cdef dict _nodepaths = {}


class Origin(Enum):
    """
    Enum: Origin of a ``Node``.

    Values:
        ``{TOP_LEFT, TOP_RIGHT, TOP_CENTER, CENTER_LEFT, CENTER, CENTER_RIGHT,
        BOTTOM_LEFT, BOTTOM_RIGHT, BOTTOM_CENTER}´´
    """
    TOP_LEFT=0
    TOP_RIGHT=1
    TOP_CENTER=2
    CENTER_LEFT=3
    CENTER=4
    CENTER_RIGHT=5
    BOTTOM_LEFT=6
    BOTTOM_RIGHT=7
    BOTTOM_CENTER=8


cdef class Node:
    """
    The ``Node`` class provides the interface to the Scenegraph. A single
    detached Node serves as the root of a Scenegraph, that can be populated,
    manipulated and queried.

    It also is meant to be subclassed for special purpose Nodes in the
    Scenegraph, such as the ``ImageNode`` class, designed to hold one or more
    images.

    .. note::
        A root Node is provided to the user in ``foolysh.app.App`` and other
        than some very specific/exotic use cases, the user should not have to
        worry about calling ``traverse()`` on the root Node, which is handled
        automatically by foolysh.
        If you're using a detached Scenegraph, make sure that you call the
        ``traverse()`` method, before accessing properties like
        ``relative_pos``, ``relative_scale`` et al.
    """
    cdef unique_ptr[_Node] thisptr

    def __cinit__(self):
        self.thisptr.reset(new _Node())
        # Reference the Node instance to keep it alive
        _nodepaths[deref(self.thisptr).get_id()] = self

    @property
    def node_id(self):
        """
        ID of the wrapped Node.
        """
        return deref(self.thisptr).get_id()

    cpdef void remove(self):
        """
        Removes the cyclic reference to the wrapped Node so it can get garbage
        collected.
        """
        _nodepaths.pop(deref(self.thisptr).get_id())

    def attach_node(self):
        """
        Attach a new child node to this Node.

        Returns:
            ``Node``
        """
        np = Node.__new__(Node)
        self._attach_node(np)
        return np

    cdef void _attach_node(self, Node np):
        cdef unique_ptr[_Node] _np
        _nodepaths.pop(deref(np.thisptr).get_id())
        _np.reset(new _Node(deref(self.thisptr).attach_node()))
        _nodepaths[deref(_np).get_id()] = np
        np.thisptr.reset(_np.release())

    def reparent_to(self, parent):
        """
        Make this a child of `parent`

        Args:
            parent: ``Node``
        """
        self._reparent_to(parent)

    cdef void _reparent_to(self, Node parent) except +:
        deref(self.thisptr).reparent_to(deref(parent.thisptr))

    def traverse(self):
        """
        Traverses the Scenegraph, starting at the root Node, relative to this.
        ..warning::
            This method is used for internal processing of the Scenegraph when
            rendering and shouldn't be called by the user. There may be some
            exotic use cases, where updating a detached Scenegraph is necessary.

        Returns:
            ``True`` if the Scenegraph had to be updated, otherwise ``False``.
        """
        return deref(self.thisptr).traverse()

    def query(self, aabb):
        """
        Query the Scenegraph at bounds `aabb`.

        Args:
            aabb: ``foolysh.tools.aabb.AABB``

        Returns:
            ``List`` of ``Node`` instances that intersect with `aabb`.
        """
        return self._query(aabb)

    cdef list _query(self, AABB aabb) except +:
        cdef SmallList[int] r = deref(self.thisptr).query(deref(aabb.thisptr))
        cdef list rl = []
        for i in range(r.size()):
            rl.append(_nodepaths[r[i]])
        return rl

    def hide(self):
        """
        Hide this and all descendants in the tree starting from this ``Node``.
        """
        deref(self.thisptr).hide()

    def show(self):
        """
        Show this ``Node``.
        """
        deref(self.thisptr).show()

    @property
    def pos(self):
        """
        ``foolysh.tools.vector2.Vector2`` position of this Node, relative to its
        parent.

        :setter:
            * ``foolysh.tools.vector2.Vector2`` -> sets the position to the
                specified ``foolysh.tools.vector2.Vector2``.
            * ``float``/``int`` -> sets the x and y coordinates to the specified
                value.
            * ``tuple`` of two ``int``/``float`` -> sets the x and y
                coordinates to the specified values.
            * ``tuple`` of ``Node`` and one of the combinations above -> sets
                the respective coordinates, relative to the specified Node.
        """
        return self._get_pos()

    @pos.setter
    def pos(self, v):
        if isinstance(v, Vector2):
            self._set_pos(v.x, v.y)
        elif isinstance(v, (int, float)):
            self._set_pos_single(v)
        elif isinstance(v, tuple) and isinstance(v[0], (int, float)) \
             and isinstance(v[1], (int, float)):
            self._set_pos(v[0], v[1])
        else:
            raise TypeError

    cdef Vector2 _get_pos(self):
        cdef _Vector2 v = deref(self.thisptr).get_pos()
        return Vector2(v[0], v[1])

    cdef void _set_pos_single(self, const double v):
        deref(self.thisptr).set_pos(v)

    cdef void _set_pos(self, const double x, const double y):
        deref(self.thisptr).set_pos(x, y)

    @property
    def scale(self):
        """
        ``tuple`` scale of this Node, relative to its parent.

        :setter:
            * ``foolysh.tools.vector2.Vector2`` -> sets the position to the
                specified ``foolysh.tools.vector2.Vector2``.
            * ``float``/``int`` -> sets the x and y scale to the specified
                value.
            * ``tuple`` of two ``int``/``float`` -> sets the x and y scale to
                the specified values.
            * ``tuple`` of ``Node`` and one of the combinations above -> sets
                the respective scale, relative to the specified Node.
        """
        s = self._get_scale()
        if s[0] == s[1]:
            return s[0]
        return s

    @scale.setter
    def scale(self, v):
        if isinstance(v, (int, float)):
            self._set_scale_single(v)
        elif isinstance(v, tuple) and isinstance(v[0], (int, float)) \
             and isinstance(v[1], (int, float)):
            self._set_scale(v[0], v[1])
        else:
            raise TypeError

    cdef tuple _get_scale(self):
        cdef Scale s = deref(self.thisptr).get_scale()
        return s.sx, s.sy

    cdef void _set_scale_single(self, const double v):
        deref(self.thisptr).set_scale(v)

    cdef void _set_scale(self, const double x, const double y):
        deref(self.thisptr).set_scale(x, y)

    @property
    def angle(self):
        """
        ``float``/``int`` rotational angle in degrees of this Node, relative to
        its parent.

        :setter:
            * ``float``/``int`` -> sets the rotational angle to the specified
                value in degrees.
            * ``tuple`` of ``Node`` and ``float``/``int`` -> sets the respective
                rotational angle, relative to the specified Node.

        .. note::
            The angle gets clamped to the range of [-180, 180].
        """
        return deref(self.thisptr).get_angle(False)

    @angle.setter
    def angle(self, v):
        if isinstance(v, (int, float)):
            self._set_angle(v)
        else:
            raise TypeError

    cdef void _set_angle(self, const double d):
        deref(self.thisptr).set_angle(d, False)

    @property
    def angle_rad(self):
        """
        ``float``/``int`` rotational angle in radians of this Node, relative to
        its parent.

        :setter:
            * ``float``/``int`` -> sets the rotational angle to the specified
                value in radians.
            * ``tuple`` of ``Node`` and ``float``/``int`` -> sets the respective
                rotational angle, relative to the specified Node.

        .. note::
            The angle gets clamped to the range of [-pi, pi].
        """
        return deref(self.thisptr).get_angle(True)

    @angle_rad.setter
    def angle_rad(self, v):
        if isinstance(v, (int, float)):
            self._set_angle_rad(v)
        else:
            raise TypeError

    cdef void _set_angle_rad(self, const double r):
        deref(self.thisptr).set_angle(r, True)

    @property
    def rotation_center(self):
        """
        ``foolysh.tools.vector2.Vector2`` rotation center of this Node, relative
        to its parent.

        :setter:
            * ``foolysh.tools.vector2.Vector2`` or ``tuple`` of two ``float``/
                ``int`` -> sets the rotation center to the specified value.
            * ``0`` -> Resets the rotation center to the default value.
        """
        return self._get_rotation_center()

    @rotation_center.setter
    def rotation_center(self, v):
        if isinstance(v, Vector2):
            self._set_rotation_center(v.x, v.y)
        elif v == 0:
            self._set_rotation_center(0.0, 0.0)
        elif isinstance(v, tuple) and isinstance(v[0], (int, float)) \
             and isinstance(v[1], (int, float)):
            self._set_rotation_center(v[0], v[1])
        else:
            raise TypeError

    cdef Vector2 _get_rotation_center(self):
        cdef _Vector2 v = deref(self.thisptr).get_rotation_center()
        return Vector2(v[0], v[1])

    cdef void _set_rotation_center(self, const double x, const double y):
        deref(self.thisptr).set_rotation_center(x, y)

    @property
    def depth(self):
        """
        ``int`` depth of this Node, relative to its parent.

        :setter:
            * ``int`` -> sets the depth to the specified value.
            * ``tuple`` of ``Node`` and ``int`` -> sets the respective
                depth, relative to the specified Node.
        """
        return deref(self.thisptr).get_depth()

    @depth.setter
    def depth(self, v):
        if isinstance(v, int):
            deref(self.thisptr).set_depth(v)
        elif isinstance(v, tuple) and isinstance(v[0], Node) \
             and isinstance(v[1], int):
            self._set_depth_node(v[0], v[1])
        else:
            raise TypeError

    cdef void _set_depth_node(self, Node other, int depth):
        deref(self.thisptr).set_depth(deref(other.thisptr), depth)

    @property
    def origin(self):
        """
        Enum: ``foolysh.scene.node.Origin`` origin of this Node.
        (default=``Origin.TOP_LEFT``)
        """
        return Origin(deref(self.thisptr).get_origin())

    @origin.setter
    def origin(self, v):
        if isinstance(v, Origin):
            self._set_origin(v.value)
        else:
            raise TypeError

    cdef void _set_origin(self, _Origin o):
        deref(self.thisptr).set_origin(o)

    @property
    def relative_pos(self):
        """
        Position, relative to the root node.
        """
        return self._get_relative_pos()

    cdef Vector2 _get_relative_pos(self):
        cdef _Vector2 v = deref(self.thisptr).get_relative_pos()
        return Vector2(v[0], v[1])

    @property
    def relative_scale(self):
        """
        Scale, relative to the root node.
        """
        s = self._get_relative_scale()
        if s[0] == s[1]:
            return s[0]
        return s

    cdef tuple _get_relative_scale(self):
        cdef Scale s = deref(self.thisptr).get_relative_scale()
        return s.sx, s.sy

    @property
    def relative_angle(self):
        """
        Angle in degrees, relative to the root node.
        """
        return deref(self.thisptr).get_relative_angle()

    @property
    def relative_depth(self):
        """
        Depth, relative to the root node.
        """
        return deref(self.thisptr).get_relative_depth()

    @property
    def size(self):
        """
        ``tuple`` of width and height of the Node.

        :setter:
            ``tuple`` of two ``float``/``int`` -> set the size to the specified
            value in asset- / base-size.
        """
        s = self._get_size()
        return s

    @size.setter
    def size(self, v):
        if isinstance(v, tuple) and isinstance(v[0], (int, float)) \
             and isinstance(v[1], (int, float)):
            self._set_size(v[0], v[1])
        else:
            raise TypeError

    cdef tuple _get_size(self):
        cdef Size s = deref(self.thisptr).get_size()
        return s.w, s.h

    cdef void _set_size(self, const double x, const double y):
        deref(self.thisptr).set_size(x, y)

    @property
    def distance_relative(self):
        """
        ``bool`` whether this Node also scales distances in the local
        coordinate system.

        :setter:
            ``bool`` -> True if scaled distances should be used or False, if
            world space distances should be used for this Node.
        """
        return deref(self.thisptr).get_distance_relative()

    @distance_relative.setter
    def distance_relative(self, v):
        if isinstance(v, bool):
            deref(self.thisptr).set_distance_relative(v)
        else:
            raise TypeError

    @property
    def relative_size(self):
        """
        ``tuple`` of width and height of the Node, relative to the root Node.
        """
        s = self._get_relative_size()
        return s

    @property
    def aabb(self):
        """
        ``foolysh.tools.aabb.AABB`` of this node.
        """
        return self._get_aabb()

    cdef AABB _get_aabb(self):
        cdef _AABB _aabb = deref(self.thisptr).get_aabb()
        return AABB(_aabb.x, _aabb.y, _aabb.hw, _aabb.hh)

    cdef tuple _get_relative_size(self):
        cdef Size s = deref(self.thisptr).get_relative_size()
        return s.w, s.h

    def __repr__(self):
        return f'{type(self).__name__}{str(self)}'

    def __str__(self):
        return f'(id:{self.node_id})'
