# distutils: language = c++
"""
Cython wrapper of the C++ Node implementation.
"""

from .cppnode cimport Node as _Node
from .cppnode cimport SceneGraphDataHandler as _SceneGraphDataHandler
from .cppnode cimport SmallList
from .cppnode cimport Scale
from .cppnode cimport Size
from .cppnode cimport Origin as _Origin
from ..tools.cppaabb cimport AABB as _AABB
from ..tools.aabb cimport AABB
from ..tools.cppvector2 cimport Vector2 as _Vector2
from ..tools.vector2 cimport Vector2
from ..tools.common import Origin

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

cdef dict _nodes = {}


cdef class SceneGraphDataHandler:
    def __cinit__(self):
        self.thisptr.reset(new _SceneGraphDataHandler())


cdef class Node:
    """
    The ``Node`` class provides the interface to the Scenegraph. A single
    detached Node serves as the root of a Scenegraph, that can be populated,
    manipulated and queried.

    It also is meant to be subclassed for special purpose Nodes in the
    Scenegraph, such as the :class:`~foolysh.scene.node.ImageNode` class,
    designed to hold one or more images.

    Args:
        name: ``str`` -> optional custom identifier of the node instance.

    .. note::
        A root Node is provided to the user in :class:`foolysh.app.App` and
        other than some very specific/exotic use cases, the user should not have
        to worry about calling ``traverse()`` on the root Node, which is handled
        automatically by foolysh.
        If you're using a detached Scenegraph, make sure that you call the
        ``traverse()`` method, before accessing properties like
        ``relative_pos``, ``relative_scale``, ...
    """

    def __cinit__(self, *args, **kwargs):
        from . import SGDH
        self._setup(SGDH)

    cdef void _setup(self, SceneGraphDataHandler sgdh):
        self.thisptr.reset(new _Node(deref(sgdh.thisptr)))
        # Reference the Node instance to keep it alive
        _nodes[deref(self.thisptr).get_id()] = self

    def __init__(self, name='Unnamed Node', *args, **kwargs):
        self.__name = name

    @property
    def name(self):
        """
        ``str`` custom identifier of the node instance.

        :setter:
            ``str`` -> change custom identifier.
        """
        return self.__name

    @name.setter
    def name(self, value):
        if not isinstance(value, str):
            raise TypeError
        self.__name = value

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
        _nodes.pop(deref(self.thisptr).get_id())

    def attach_node(self, name='Unnamed Node'):
        """
        Attach a new child node to this Node.

        Args:
            name: ``str`` -> optional custom identifier of the node instance.

        Returns:
            ``Node``
        """
        np = Node.__new__(Node)
        np.name = name
        self._attach_node(np)
        return np

    cdef void _attach_node(self, Node np):
        cdef unique_ptr[_Node] _np
        _nodes.pop(deref(np.thisptr).get_id())
        _np.reset(new _Node(deref(self.thisptr).attach_node()))
        _nodes[deref(_np).get_id()] = np
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

        .. warning::
            This method is used for internal processing of the Scenegraph when
            rendering and shouldn't be called by the user. There may be some
            exotic use cases, where updating a detached Scenegraph could be
            useful.

        Returns:
            ``True`` if the Scenegraph had to be updated, otherwise ``False``.
        """
        return deref(self.thisptr).traverse()

    def query(self, aabb, depth_sorted=True):
        """
        Query the Scenegraph at bounds `aabb`.

        Args:
            aabb: :class:`foolysh.tools.aabb.AABB`

        Returns:
            ``list`` of ``Node`` instances that intersect with `aabb`.
        """
        return self._query(aabb, depth_sorted)

    cdef list _query(self, AABB aabb, bint depth_sorted) except +:
        cdef SmallList[int] r = deref(self.thisptr).query(
            deref(aabb.thisptr),
            depth_sorted
        )
        cdef list rl = []
        for i in range(r.size()):
            rl.append(_nodes[r[i]])
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
        :class:`foolysh.tools.vector2.Vector2` position of this Node, relative
        to its parent.

        :setter:
            * :class:`foolysh.tools.vector2.Vector2` -> sets the position to the
                specified :class:`foolysh.tools.vector2.Vector2`.
            * ``float``/``int`` -> sets the x and y coordinates to the specified
                value.
            * ``tuple`` of two ``int``/``float`` -> sets the x and y
                coordinates to the specified values.
            * ``tuple`` of ``Node`` and one of the combinations above -> sets
                the respective coordinates, relative to the specified Node.

        .. warning::
            The returned :class:`foolysh.tools.vector2.Vector2` is a copy of the
            current pos. Any changes made to it are independent from the
            internally stored position!            
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
        elif isinstance(v, tuple) and isinstance(v[0], Node):
            if len(v) == 2 and isinstance(v[1], (int, float)):
                self._set_pos_relative(v[0], v[1], v[1])
            elif len(v) == 2 and isinstance(v[1], Vector2):
                self._set_pos_relative(v[0], v[1].x, v[1].y)
            elif len(v) == 3 and isinstance(v[1], (int, float)) \
                 and isinstance(v[2], (int, float)):
                self._set_pos_relative(v[0], v[1], v[2])
            else:
                raise TypeError
        else:
            raise TypeError

    @property
    def x(self):
        """
        X-coordinate of this Node, relative to its parent.

        :setter:
            * ``float``/``int`` -> sets the x-coordinate to the specified value.
        """
        return self._get_pos().x

    @x.setter
    def x(self, value):
        if not isinstance(value, (float, int)):
            raise TypeError
        p = self._get_pos()
        self._set_pos(value, p.y)

    @property
    def y(self):
        """
        Y-coordinate of this Node, relative to its parent.

        :setter:
            * ``float``/``int`` -> sets the y-coordinate to the specified value.
        """
        return self._get_pos().y

    @y.setter
    def y(self, value):
        if not isinstance(value, (float, int)):
            raise TypeError
        p = self._get_pos()
        self._set_pos(p.x, value)

    @property
    def xy(self):
        """
        XY-coordinates ``tuple`` of this Node, relative to its parent.

        :setter:
            * ``Tuple[int/float, int/float]`` -> sets the xy-coordinates to the 
                specified value.
        """
        p = self._get_pos()
        return p.x, p.y

    @xy.setter
    def xy(self, value):
        if not isinstance(value, tuple):
            raise TypeError
        if len(value) != 2:
            raise ValueError('Expected tuple of length 2.')
        x, y = value
        if not isinstance(x, (float, int)) or not isinstance(y, (float, int)):
            raise TypeError
        self._set_pos(x, y)

    def get_pos_node(self, other):
        """
        Retrieve the position, relative to `other`.

        Args:
            other: The Node used as reference.
        Returns:
            :class:`foolysh.tools.vector2.Vector2` Position relative to another
            Node.
        """
        return self._get_pos_node(other)

    cdef Vector2 _get_pos(self):
        cdef _Vector2 v = deref(self.thisptr).get_pos()
        return Vector2(v[0], v[1])

    cdef void _set_pos_single(self, const double v):
        deref(self.thisptr).set_pos(v)

    cdef void _set_pos(self, const double x, const double y):
        deref(self.thisptr).set_pos(x, y)

    cdef void _set_pos_relative(
        self,
        Node other,
        const double x,
        const double y
    ):
        deref(self.thisptr).set_pos(deref(other.thisptr), x, y)

    cdef Vector2 _get_pos_node(self, Node other):
        cdef _Vector2 v = deref(self.thisptr).get_pos(deref(other.thisptr))
        return Vector2(v[0], v[1])

    @property
    def scale(self):
        """
        ``tuple`` scale of this Node, relative to its parent.

        :setter:
            * :class:`foolysh.tools.vector2.Vector2` -> sets the position to the
                specified :class:`foolysh.tools.vector2.Vector2`.
            * ``float``/``int`` -> sets the x and y scale to the specified
                value.
            * ``tuple`` of two ``int``/``float`` -> sets the x and y scale to
                the specified values.
            * ``tuple`` of ``Node`` and one of the combinations above -> sets
                the respective scale, relative to the specified Node.
        """
        s = self._get_scale()
        return s

    @scale.setter
    def scale(self, v):
        if isinstance(v, (int, float)):
            self._set_scale_single(v)
        elif isinstance(v, tuple) and isinstance(v[0], (int, float)) \
             and isinstance(v[1], (int, float)):
            self._set_scale(v[0], v[1])
        elif isinstance(v, tuple) and isinstance(v[0], Node):
            if len(v) == 3 and isinstance(v[1], (int, float)) \
                 and isinstance(v[2], (int, float)):
                self._set_scale_node(v[0], v[1], v[2])
            elif len(v) == 2 and isinstance(v[1], (int, float)):
                self._set_scale_node(v[0], v[1], v[1])
            else:
                raise TypeError
        else:
            raise TypeError

    @property
    def scale_single(self):
        """
        ``int``/``float`` scale of this Node, relative to its parent. Raises a
        ``TypeError`` if the scale is asimmetrical.
        """
        s = self._get_scale()
        if s[0] == s[1]:
            return s[0]
        raise TypeError('Requested Node scale as single element, but '
                        'the Node has an asymmetrical scale.')

    def get_scale_single_node(self, other):
        """
        Retrieve the single part scale, relative to `other`. Raises a
        ``TypeError`` if the scale is asimmetrical.

        Args:
            other: The Node used as reference.
        Returns:
            ``float``/``int`` Scale relative to another Node.
        """
        s = self._get_scale_node(other)
        if s[0] == s[1]:
            return s[0]
        raise TypeError('Requested Node scale as single element, but '
                        'the Node has an asymmetrical scale.')

    def get_scale_node(self, other):
        """
        Retrieve the two part scale, relative to `other`.

        Args:
            other: The Node used as reference.
        Returns:
            ``tuple`` of two ``float``/``int`` Scale relative to another Node.
        """
        s = self._get_scale_node(other)
        return s

    cdef tuple _get_scale(self):
        cdef Scale s = deref(self.thisptr).get_scale()
        return s.sx, s.sy

    cdef tuple _get_scale_node(self, Node other):
        cdef Scale s = deref(self.thisptr).get_scale(deref(other.thisptr))
        return s.sx, s.sy

    cdef void _set_scale_single(self, const double v):
        deref(self.thisptr).set_scale(v)

    cdef void _set_scale(self, const double x, const double y):
        deref(self.thisptr).set_scale(x, y)

    cdef void _set_scale_node(self, Node other, const double x, const double y):
        deref(self.thisptr).set_scale(deref(other.thisptr), x, y)

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
        elif isinstance(v, tuple) and isinstance(v[0], Node) \
             and isinstance(v[1], (int, float)):
             self._set_angle_node(v[0], v[1])
        else:
            raise TypeError

    def get_angle_node(self, other):
        """
        Retrieve the rotational angle in degrees, relative to `other`.

        Args:
            other: The Node used as reference.
        Returns:
            ``float`` rotational angle relative to another Node.
        """
        return self._get_angle_node(other)

    cdef double _get_angle_node(self, Node other):
        return deref(self.thisptr).get_angle(deref(other.thisptr), False)

    cdef void _set_angle(self, const double d):
        deref(self.thisptr).set_angle(d, False)

    cdef void _set_angle_node(self, Node other, const double r):
        deref(self.thisptr).set_angle(deref(other.thisptr), r, False)

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
        elif isinstance(v, tuple) and isinstance(v[0], Node) \
             and isinstance(v[1], (int, float)):
             self._set_angle_rad_node(v[0], v[1])
        else:
            raise TypeError

    def get_angle_rad_node(self, other):
        """
        Retrieve the rotational angle in radians, relative to `other`.

        Args:
            other: The Node used as reference.
        Returns:
            ``float`` rotational angle relative to another Node.
        """
        return self._get_angle_rad_node(other)

    cdef double _get_angle_rad_node(self, Node other):
        return deref(self.thisptr).get_angle(deref(other.thisptr), True)

    cdef void _set_angle_rad(self, const double r):
        deref(self.thisptr).set_angle(r, True)

    cdef void _set_angle_rad_node(self, Node other, const double r):
        deref(self.thisptr).set_angle(deref(other.thisptr), r, True)

    @property
    def rotation_center(self):
        """
        :class:`foolysh.tools.vector2.Vector2` rotation center of this Node,
        relative to its parent.

        :setter:
            * :class:`foolysh.tools.vector2.Vector2` or ``tuple`` of two
                ``float`` / ``int`` -> sets the rotation center to the specified
                value.
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
        :class:`foolysh.tools.common.Origin` origin of this Node. (default = ``Origin.TOP_LEFT``)
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
        return self._get_relative_scale()

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
            ``bool`` -> Tr<double> args[0][0]ue if scaled distances should be used or False, if
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
        :class:`foolysh.tools.aabb.AABB` of this node.
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
        return f'({self.name}:{self.node_id})'


cdef class ImageNode(Node):
    """
    Node type, that additionally holds a :attr:`image` property to the
    :class:`Node`.
    """
    cdef list _images
    cdef int _current_index

    def __init__(self, name=None, image=None):
        super(ImageNode, self).__init__(name=name)
        self._images = []            # type: List[str]
        self._current_index = -1
        if image is not None:
            self.add_image(image)

    @property
    def image(self):
        # type: () -> str
        """The current active image."""
        if self._current_index == -1:
            raise RuntimeError('No image added yet.')
        return self._images[self._current_index]

    @property
    def index(self):
        # type: () -> int
        """The index of the currently active image."""
        if self._current_index == -1:
            raise RuntimeError('No image added yet.')
        return self._current_index

    @index.setter
    def index(self, value):
        # type: (Optional[int]) -> None
        """
        Loads the first image or optionally the image with index ``item``
        """
        if not isinstance(value, int):
            raise TypeError
        if not -1 < value < len(self._images):
            raise IndexError('Invalid index')
        if value != self._current_index:
            self._current_index = value

    def add_image(self, image):
        # type: (str) -> int
        """
        Add an image to the ``ImageNode``.

        Args:
            image: ``str`` -> the image path relative to the asset directory.

        Returns:
            ``int`` -> the image index.
        """
        self._images.append(image)
        idx = len(self._images) - 1
        if idx == 0:
            self.index = idx
        return idx
