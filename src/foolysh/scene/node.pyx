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
from ..tools.cppvec2 cimport Vec2 as _Vec2
from ..tools.vec2 cimport Vec2
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
cdef list _need_size = []


def unsized_nodes():
    """
    Returns a list of nodes that have no size but represent either an image or
    text.

    .. warning::
        The list is cleared after each call to this function!
    """
    nodes = [_nodes[i] for i in _need_size if i in _nodes]
    _need_size.clear()
    return nodes


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

    def __init__(self, name=None, *args, **kwargs):
        self.__name = name or 'Unnamed Node'

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

    @property
    def parent_id(self):
        """
        ID of the parent Node. `-1` if called on a root Node.
        """
        return deref(self.thisptr).get_parent_id()

    cpdef void remove(self):  # TODO: Fix removing of nodes
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

    def attach_image_node(self, name='Unnamed Node', image=None):
        """
        Attach a new child image node to this Node.

        Args:
            name: ``str`` -> optional custom identifier of the node instance.
            image: ``str`` -> optional image path.

        Returns:
            ``ImageNode``
        """
        np = ImageNode(name, image)
        self._attach_node(np)
        return np

    def attach_text_node(self, name='Unnamed Node', text='', font=None,
                         font_size=0.05, text_color=(255, 255, 255, 255),
                         align='left', spacing=0, multiline=False, **unused_kw):
        """
        Attach a new child text node to this Node.

        Args:
            name: ``str`` -> optional custom identifier of the node instance.
            text: ``str`` -> optional text to display.
            font: ``str`` -> optional font.
            size: ``float`` -> optional font size.
            color: ``4 value Tuple[int]`` -> optional font color.
            align: ``str`` -> optional text alignment on multiline text.
            spacing: ``int`` -> optional spacing between lines.
            multiline: ``bool`` -> optional whether the text spans across
                multiple lines.

        Returns:
            ``TextNode``
        """
        np = TextNode(name, text, font, font_size, text_color, align, spacing,
                      multiline)
        self._attach_node(np)
        return np

    def _on_origin_change(self):
        pass

    cdef void _attach_node(self, Node np):
        deref(np.thisptr).reparent_to(deref(self.thisptr))

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
        cdef SmallList[size_t] r = deref(self.thisptr).query(
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
    def hidden(self):
        """Whether the Node is currently hidden in the scene."""
        return deref(self.thisptr).hidden()

    @property
    def pos(self):
        """
        :class:`foolysh.tools.vec2.Vec2` position of this Node, relative
        to its parent.

        :setter:
            * :class:`foolysh.tools.vec2.Vec2` -> sets the position to the
                specified :class:`foolysh.tools.vec2.Vec2`.
            * ``float``/``int`` -> sets the x and y coordinates to the specified
                value.
            * ``tuple`` of two ``int``/``float`` -> sets the x and y
                coordinates to the specified values.
            * ``tuple`` of ``Node`` and one of the combinations above -> sets
                the respective coordinates, relative to the specified Node.

        .. warning::
            The returned :class:`foolysh.tools.vec2.Vec2` is a copy of the
            current pos. Any changes made to it are independent from the
            internally stored position!
        """
        return self._get_pos()

    @pos.setter
    def pos(self, v):
        if isinstance(v, Vec2):
            self._set_pos(v.x, v.y)
        elif isinstance(v, (int, float)):
            self._set_pos_single(v)
        elif isinstance(v, tuple) and isinstance(v[0], (int, float)) \
             and isinstance(v[1], (int, float)):
            self._set_pos(v[0], v[1])
        elif isinstance(v, tuple) and isinstance(v[0], Node):
            if len(v) == 2 and isinstance(v[1], (int, float)):
                self._set_pos_relative(v[0], v[1], v[1])
            elif len(v) == 2 and isinstance(v[1], Vec2):
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
            :class:`foolysh.tools.vec2.Vec2` Position relative to another
            Node.
        """
        return self._get_pos_node(other)

    cdef Vec2 _get_pos(self):
        cdef _Vec2 v = deref(self.thisptr).get_pos()
        return Vec2(v[0], v[1])

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

    cdef Vec2 _get_pos_node(self, Node other):
        cdef _Vec2 v = deref(self.thisptr).get_pos(deref(other.thisptr))
        return Vec2(v[0], v[1])

    @property
    def scale(self):
        """
        ``tuple`` scale of this Node, relative to its parent.

        :setter:
            * :class:`foolysh.tools.vec2.Vec2` -> sets the position to the
                specified :class:`foolysh.tools.vec2.Vec2`.
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
        :class:`foolysh.tools.vec2.Vec2` rotation center of this Node,
        relative to its parent.

        :setter:
            * :class:`foolysh.tools.vec2.Vec2` or ``tuple`` of two
                ``float`` / ``int`` -> sets the rotation center to the specified
                value.
            * ``0`` -> Resets the rotation center to the default value.
        """
        return self._get_rotation_center()

    @rotation_center.setter
    def rotation_center(self, v):
        if isinstance(v, Vec2):
            self._set_rotation_center(v.x, v.y)
        elif v == 0:
            self._set_rotation_center(0.0, 0.0)
        elif isinstance(v, tuple) and isinstance(v[0], (int, float)) \
             and isinstance(v[1], (int, float)):
            self._set_rotation_center(v[0], v[1])
        else:
            raise TypeError

    cdef Vec2 _get_rotation_center(self):
        cdef _Vec2 v = deref(self.thisptr).get_rotation_center()
        return Vec2(v[0], v[1])

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
            self._on_origin_change()
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

    cdef Vec2 _get_relative_pos(self):
        cdef _Vec2 v = deref(self.thisptr).get_relative_pos()
        return Vec2(v[0], v[1])

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
            if self.node_id in _need_size:
                _need_size.pop(_need_size.index(self.node_id))
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

    def propagate_dirty(self):
        deref(self.thisptr).propagate_dirty()

    def __repr__(self):
        return f'{type(self).__name__}{str(self)}'

    def __str__(self):
        return f'({self.name}:{self.node_id})'


cdef class ImageNode(Node):
    """
    Node type, that additionally holds a :attr:`image` property.
    """
    cdef list _images
    cdef int _current_index
    cdef bint _tiled

    def __init__(self, name=None, image=None, tiled=False):
        super(ImageNode, self).__init__(name=name)
        self._images = []            # type: List[str]
        self._current_index = -1
        self._tiled = tiled
        if image is not None:
            self.add_image(image)
        _need_size.append(self.node_id)

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
            self.propagate_dirty()

    @property
    def tiled(self):
        # type: () -> bool
        """Whether this is a tiled image that spans the screen."""
        return self._tiled

    @tiled.setter
    def tiled(self, value):
        # type: (bool) -> None
        if not isinstance(value, bool):
            raise TypeError
        self._tiled = value

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

    def clear_images(self):
        """Removes all images stored in this node."""
        self._images = []
        self._current_index = -1

    def __getitem__(self, item):
        return self._images[item]

    def __setitem__(self, item, value):
        if self._images[item] != value:
            self._images[item] = value
            if item == self._current_index:
                self.propagate_dirty()


cdef class TextNode(Node):
    """
    Node type, that additionally holds a text property.

    Args:
        name: ``Optional[str]``
        text: ``Optional[str]``
        font: ``Optional[str]`` -> path to the font, relative to the `asset_dir`
            config value.
        font_size: ``Optional[float]``
        text_color: ``Optional[Tuple[int, int, int, int]]``
        align: ``Optional[str]`` -> text alignment on multiline text.
        spacing: ``Optional[float]`` -> row spacing
        multiline: ``Optional[bool]`` -> whether the text spans multiple lines.
    """
    cdef str _text
    cdef str _font
    cdef float _font_size
    cdef tuple _text_color
    cdef str _align
    cdef int _spacing
    cdef bint _multiline

    def __init__(self, name=None, text='', font=None, font_size=0.05,
                 text_color=(255, 255, 255, 255), align='left', spacing=0,
                 multiline=False, *args, **kwargs):
        super(TextNode, self).__init__(name=name)
        self._text = ''
        self._font = ''
        self._font_size = 0
        self._text_color = ()
        self._align = ''
        self._spacing = 0
        self._multiline = False
        self.text = text
        self.font = font or ''
        self.font_size = font_size
        self.text_color = text_color
        self.align = align
        self.spacing = spacing
        self.multiline = multiline
        _need_size.append(self.node_id)

    @property
    def hashkey(self):
        return hash(f'{self._text}{self._font}{self._font_size}{self._text_color}'
                    f'{self._align}{self._spacing}{self._multiline}')

    @property
    def text(self):
        # type: () -> str
        """The current text."""
        return self._text

    @text.setter
    def text(self, value):
        # type: (str) -> None
        if not isinstance(value, str):
            raise TypeError
        if self._text != value:
            self._text = value
            self.propagate_dirty()
            if self.node_id not in _need_size:
                _need_size.append(self.node_id)

    @property
    def font(self):
        # type: () -> str
        """The current asset path to the font."""
        if self._font == '':
            raise RuntimeError('No font set yet.')
        return self._font

    @font.setter
    def font(self, value):
        # type: (str) -> None
        if not isinstance(value, str):
            raise TypeError
        self._font = value

    @property
    def font_size(self):
        # type: () -> float
        """The font size."""
        return self._font_size

    @font_size.setter
    def font_size(self, value):
        # type: (float) -> None
        if not isinstance(value, (float, int)):
            raise TypeError
        if value <= 0.0:
            raise ValueError('Expected positive, non zero float.')
        self._font_size = float(value)

    @property
    def text_color(self):
        # type: () -> Tuple[int, int, int, int]
        """
        The text color.

        :setter:
            * ``int`` -> sets all components of the color to the specified
                value.
            * 3-/4-``tuple`` of ``int`` -> RGB or RGBA color. The alpha value
                defaults to 255, if RGB only is provided.
        """
        return self._text_color

    @text_color.setter
    def text_color(self, value):
        # type: (Union[int, Tuple[int, ...]]) -> None
        if isinstance(value, int):
            value = (value, ) * 3 + (255, )
        elif isinstance(value, tuple):
            components = len(value)
            if components == 3:
                value = value + (255, )
            elif components != 4:
                raise ValueError('Expected tuple of length 3 or 4.')
            for c in value:
                if not isinstance(c, int):
                    raise TypeError
                if not 0 <= c <= 255:
                    raise ValueError('Expected value in range (0..255).')
        else:
            raise TypeError
        self._text_color = value

    @property
    def align(self):
        # type: () -> str
        """If `multiline`, align the text 'left', 'center', 'right'."""
        return self._align

    @align.setter
    def align(self, value):
        # type: (str) -> None
        if not isinstance(value, str):
            raise TypeError
        if value not in ('left', 'center', 'right'):
            raise ValueError('Expected one of "left", "center", "right".')
        self._align = value

    @property
    def spacing(self):
        # type: () -> float
        """If `multiline`, spacing between lines."""
        return self._spacing

    @spacing.setter
    def spacing(self, value):
        # type: (float) -> None
        if not isinstance(value, (int, float)):
            raise TypeError
        if value < 0:
            raise ValueError('Expected positive float.')
        self._spacing = value

    @property
    def multiline(self):
        # type: () -> bool
        """Whether the text is multi or single line"""
        return self._multiline

    @multiline.setter
    def multiline(self, value):
        # type: (bool) -> None
        if not isinstance(value, bool):
            raise TypeError
        self._multiline = value
