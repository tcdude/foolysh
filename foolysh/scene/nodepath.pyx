"""
Cython implementation of NodePath class.
"""

from ..tools import spriteloader
from ..scene import node
# noinspection PyUnresolvedReferences
from ..tools.vector cimport Point
# noinspection PyUnresolvedReferences
from ..tools.vector cimport Vector
# noinspection PyUnresolvedReferences
from ..tools.quadtree cimport Quadtree
# noinspection PyUnresolvedReferences
from ..tools.quadtree cimport quadtree_from_pairs
# noinspection PyUnresolvedReferences
from ..tools.aabb cimport AABB

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

# noinspection PyAttributeOutsideInit
cdef class NodePath:
    """
        Structural part of the Scene Graph.
        A scene starts at a root NodePath, that maintains a QuadTree for fast access
        of NodePath and Node objects. New NodePath objects can be created by calling
        the ``attach_new_node_path()`` method of the appropriate "Parent" NodePath.
        A NodePath can hold a Node type object and keeps track of position, scale,
        rotation and depth. The properties propagate from the root NodePath down to
        all children in the graph.

        :param name: Optional name
        :param center: Specifies where the origin lies (CENTER=0, TOP_LEFT=1,
            BOTTOM_RIGHT=2) as int (default=TOP_LEFT)
        :param visible: Whether the NodePath is visible (default=True)
        :param position: Optional Point -> position offset relative to its parent.
        :param angle: Optional float -> angle of rotation in degrees relative to its
            parent.
        :param scale: Optional float -> scale relative to its parent.
        :param depth: Optional int -> depth relative to its parent.
        :param parent: Optional NodePath -> If specified, the instance will be a
            child of ``parent`` and inherit position, scale, rotation and depth.
        :param max_level: Optional int -> maximum levels of nesting of the quadtree.

        """

    def __init__(
            self,
            name=None,
            center=Origin.TOP_LEFT,
            visible=True,
            position=Point(0, 0),
            angle=0.0,
            scale=1.0,
            depth=1,
            parent=None,
            max_level=8
    ):
        self._np_name = name or 'Unnamed NodePath'
        self._node = node.Node(self)
        self.update_relative()

    def __cinit__(
            self,
            name=None,
            center=Origin.TOP_LEFT,
            visible=True,
            position=Point(0, 0),
            angle=0.0,
            scale=1.0,
            depth=1,
            parent=None,
            max_level=8
    ):
        cdef NodePath _parent
        self._center = center
        self._visible = visible
        self._position = position
        self._angle = angle
        self._scale = scale
        self._depth = depth
        self._rotation_center = None
        self._rel_position = Point()
        self._rel_angle = 0.0
        self._rel_scale = 1.0
        self._rel_depth = 0
        self._asset_pixel_ratio = 1
        self._dummy_size = None
        self._children = []
        self._tags = {}
        self._dirty = True
        self._max_level = max_level
        self._quadtree = None
        self._sprite_loader = None
        if parent is None:
            self._is_root = True
            self._parent = None
        else:
            _parent = parent
            self._is_root = False
            self._parent = parent
            # noinspection PyProtectedMember
            if not _parent._dirty:
                _parent._dirty = True

    @property
    def root_nodepath(self):
        if self.is_root:
            return self
        p = self.parent
        while not p.is_root:
            p = p.parent
        return p

    @property
    def parent(self):
        if self._parent is None:
            return None
        return self._parent

    @property
    def is_root(self):
        return self._is_root

    @property
    def visible(self):
        """``bool``"""
        return self._visible

    @visible.setter
    def visible(self, value):
        if isinstance(value, bool):
            if value != self._visible:
                self._visible = value
                self.dirty = True
        else:
            raise TypeError('visible must be of type bool')

    @property
    def relative_position(self):
        """``engine.tools.vector.Point``"""
        return self._rel_position

    @property
    def relative_angle(self):
        """``float``"""
        return self._rel_angle

    @property
    def relative_scale(self):
        """``float``"""
        return self._rel_scale

    @property
    def relative_depth(self):
        """``int``"""
        return self._rel_depth

    @property
    def position(self):
        """``engine.tools.vector.Point``"""
        return self._position

    @position.setter
    def position(self, value):
        if isinstance(value, (Point, Vector)):
            self._position = value
        elif isinstance(value, tuple) and len(value) == 2 and \
                isinstance(value[0], float) and isinstance(value[1], float):
            self._position = Point(value)
        else:
            raise TypeError('position must be of type Point, Vector or '
                            'Tuple[float, float]')
        self.dirty = True

    @property
    def angle(self):
        """``float``"""
        return self._angle

    @angle.setter
    def angle(self, value):
        if isinstance(value, (int, float)):
            self._angle = float(value)
            self.dirty = True
        else:
            raise TypeError('rotation must be of type float or int')

    @property
    def scale(self):
        """``float``"""
        return self._scale

    @scale.setter
    def scale(self, value):
        if isinstance(value, (int, float)):
            self._scale = float(value)
            self.dirty = True
        else:
            raise TypeError('scale must be of type float or int')

    @property
    def depth(self):
        """``int``"""
        return self._depth

    @depth.setter
    def depth(self, value):
        if isinstance(value, int):
            self._depth = value
            self.dirty = True
        else:
            raise TypeError('depth must be of type int')

    @property
    def center(self):
        """``int`` in ``engine.tools.CENTER/.TOP_LEFT/.BOTTOM_RIGHT``"""
        return self._center

    @center.setter
    def center(self, value):
        if isinstance(value, Origin):
            self._center = value
            self.dirty = True
        else:
            raise TypeError('must be of type nodepath.Origin')

    @property
    def rotation_center(self):
        """``Union[None, Tuple[int, int]]``"""
        return self._rotation_center

    @rotation_center.setter
    def rotation_center(self, value):
        if isinstance(value, tuple) and len(value) == 2 and \
                isinstance(value[0], int) and isinstance(value[1], int):
            self._rotation_center = value
        elif value is None:
            self._rotation_center = None
        else:
            raise TypeError('expected Tuple[int, int] or None')
        self.dirty = True

    @property
    def asset_pixel_ratio(self):
        """``int``"""
        if self.is_root:
            return self._asset_pixel_ratio
        return self.root_nodepath.asset_pixel_ratio

    @asset_pixel_ratio.setter
    def asset_pixel_ratio(self, value):
        if self.is_root:
            if isinstance(value, int) and value > 0:
                if value > 0:
                    self._asset_pixel_ratio = value
                    self.dirty = True
                else:
                    raise ValueError('expected int > 0')
            else:
                raise TypeError('expected type int')
        else:
            raise ValueError('asset_pixel_ratio property can only be set on a '
                             'NodePath marked as root')

    @property
    def sprite_loader(self):
        """``engine.tools.spriteloader.SpriteLoader``"""
        if self.is_root:
            return self._sprite_loader
        return self.root_nodepath.sprite_loader

    @sprite_loader.setter
    def sprite_loader(self, value):
        if self._is_root:
            if isinstance(value, spriteloader.SpriteLoader):
                self._sprite_loader = value
                self.dirty = True
            else:
                raise TypeError('expected type SpriteLoader')
        else:
            raise ValueError('sprite_loader property can only be set on a '
                             'NodePath marked as root')

    @property
    def size(self):
        """``Tuple[float, float]``"""
        if self.node.size is None:
            if self._dummy_size is None:
                return 0.0, 0.0
            return self._dummy_size
        ns = Vector(self.node.size) / self.asset_pixel_ratio
        return tuple(ns * self._rel_scale)

    @property
    def children(self):
        """``List[NodePath]``"""
        return self._children

    @property
    def node(self):
        """``Node``"""
        return self._node

    @node.setter
    def node(self, value):
        if isinstance(value, node.Node):
            self._node = value
            self.dirty = True
        else:
            raise TypeError(f'expected type Node, got {type(value).__name__}')

    @property
    def quadtree(self):
        """``engine.tools.quadtree.Quadtree``"""
        if self.is_root:
            return self._quadtree
        return self.root_nodepath.quadtree

    @quadtree.setter
    def quadtree(self, value):
        if self.is_root:
            if isinstance(value, Quadtree):
                self._quadtree = value
                self.dirty = True
            else:
                raise TypeError('expected type Quadtree')
        else:
            raise ValueError('quadtree property can only be set on a NodePath '
                             'marked as root')

    @property
    def dirty(self):
        """``bool``"""
        return self._dirty

    @dirty.setter
    def dirty(self, value):
        if not isinstance(value, bool):
            raise TypeError('expected bool')
        self._dirty = value
        if value:
            self.propagate_dirty()

    # noinspection PyProtectedMember,PyUnresolvedReferences
    cdef void propagate_dirty(self):
        """
        Propagates ``dirty=True`` to all ``NodePath`` instances that are below 
        self and sets ``dirty=True`` on the direct way to the ``NodePath`` 
        marked as root.
        """
        cdef long n, c
        cdef list parents = [self]
        cdef list new_parents
        cdef NodePath parent

        while parents:  # propagate to all sub node paths
            new_parents = []
            for n in range(len(parents)):
                for c in range(len(parents[n].children)):
                    new_parents.append(parents[n].children[c])
                    parents[n].children[c]._dirty = True
            parents = new_parents

        parent = self._parent
        while parent is not None:   # propagate to all parent node paths
            parent._dirty = True
            parent = parent._parent


    cpdef void set_dummy_size(self, size):
        """
        Sets a dummy size for the NodePath.

        :param size: ``Union[Vector, Point, Tuple[float, float]]``
        """
        if not isinstance(size, tuple):
            size = tuple(size)
        if len(size) == 2 and isinstance(size[0], (int, float)) \
                and isinstance(size[1], (int, float)):
            self._dummy_size = size
            self.dirty = True
        else:
            raise TypeError('expected Vector, Point or Tuple[float, float]')

    cpdef tuple update_relative(self):
        """
        Update the relative attributes in respect to ``parent``.

        :return: ``4-Tuple[float, float, float, float]`` -> bounding box of the
            ``NodePath``.
        """
        return self._update_relative()

    # noinspection PyProtectedMember
    cdef tuple _update_relative(self):
        cdef Point offset, rel_pos
        cdef tuple box
        cdef NodePath parent

        if self._center == Origin.CENTER:
            offset = Point(self.size) / -2
        elif self._center == TOP_LEFT:
            offset = Point()
        elif self._center == BOTTOM_RIGHT:
            offset = Point() - Point(self.size)
        else:
            raise ValueError('invalid value in property NodePath.center')

        if self._is_root:
            self._rel_position = self._position + offset
            self._rel_angle = self._angle
            self._rel_scale = self._scale
            self._rel_depth = self._depth
        else:
            parent = self._parent
            if parent.relative_angle:
                rel_pos = self._position.rotate(
                    parent._rel_angle
                ).aspoint()
            else:
                rel_pos = self._position
            self._rel_position = (
                    parent._rel_position + rel_pos + offset
            )
            self._rel_angle = parent._rel_angle + self._angle
            self._rel_scale = parent._rel_scale * self._scale
            self._rel_depth = parent._rel_depth + self._depth
        box = tuple(self._rel_position) + tuple(
            self._rel_position + Point(self.size))
        return box

    cpdef bint traverse(self):
        """
        Traverse the scene graph to update relative properties and update the
        quadtree of the root NodePath.

        .. warning::
            This will raise a ``ValueError`` if called from a node not marked
            as ``NodePath.is_root``.
        """
        if not self.is_root:
            raise ValueError('NodePath.traverse() can only be called from a '
                             'NodePath marked as NodePath.is_root')
        if not self.dirty:
            return False
        return self._traverse()

    # noinspection PyProtectedMember,PyUnresolvedReferences
    cdef bint _traverse(self):
        cdef tuple box
        cdef long n
        cdef list node_paths, quadtree_pairs
        cdef NodePath node_path

        node_paths = [self]
        quadtree_pairs = []
        while node_paths:
            new_node_paths = []
            for n in range(len(node_paths)):
                node_path = node_paths[n]
                if node_path._dirty:
                    box = node_path._update_relative()
                    if (box[2] - box[0]) * (box[3] - box[1]) > 0:
                        quadtree_pairs.append((AABB(box), node_path))
                    node_path._dirty = False
                    new_node_paths += node_path._children
            node_paths = new_node_paths

        qt = quadtree_from_pairs(quadtree_pairs, self._max_level)
        if qt is not None:
            self.quadtree = qt
            return True
        return False

    cpdef bint reparent_to(self, NodePath new_parent):
        """
        Reparent this instance to another parent NodePath.

        :param new_parent: ``NodePath`` -> The new parent.
        :return: ``bool`` -> success.
        """
        if isinstance(new_parent, NodePath):
            if self.parent is not None:
                self.parent.remove_node_path(self)
            self._parent = new_parent
            # noinspection PyUnresolvedReferences
            self.parent.children.append(self)
            self._is_root = False
            self.dirty = True
            return True
        return False

    cpdef NodePath attach_new_node_path(
            self,
            name=None,
            center=None,
            visible=True,
            position=Point(0, 0),
            angle=0.0,
            scale=1.0,
            depth=0
    ):
        """
        Attach and return a new child ``NodePath`` to this instance.

        :param name: Optional ``str`` -> name of the new ``NodePath``
        :param center: Optional ``int`` -> origin of the new ``NodePath``
        :param visible: Optional ``bool``
        :param position: Optional ``engine.tools.vector.Point``
        :param angle: Optional ``float``
        :param scale: Optional ``float``
        :param depth: Optional ``int``
        :return: ``engine.scene.nodepath.NodePath``
        """
        cdef NodePath np

        np = NodePath(
            name=name,
            center=center or self.center,
            visible=visible,
            position=position,
            angle=angle,
            scale=scale,
            depth=depth,
            parent=self
        )
        self._dirty = True
        self.children.append(np)
        return np

    cpdef list query(self, q_aabb, bint overlap=True):
        """
        Return a list of ``NodePath`` instances that lie within or overlap with
        the given ``AABB``, ``Point``, ``Vector`` or ``2-Tuple[float, float]``.

        :param q_aabb: ``engine.tools.aabb.AABB`` -> bounding box to query.
        :param overlap: Optional ``bool`` -> whether to include or exclude
            overlapping ``NodePath`` instances.
        :return: ``List[NodePath]``
        """
        if self.is_root:
            if self.dirty:
                self.traverse()
            if self.quadtree is not None:
                return self.quadtree.get_items(q_aabb, overlap)
            raise ValueError('unable to populate a Quadtree.')
        return self.root_nodepath.query(q_aabb, overlap)

    cpdef bint remove_node_path(self, NodePath np):
        """
        Removes the passed ``NodePath``. Return ``True`` if ``np`` is a child,
        otherwise ``False``.

        :param np: ``NodePath``
        :return: ``bool``
        """
        if np in self.children:
            self.children.pop(self.children.index(np))
            self.dirty = True
            return True
        return False

    cpdef pop(self, item):
        return self._tags.pop(item)

    def __getitem__(self, item):
        return self._tags[item]

    def __setitem__(self, key, value):
        self._tags[key] = value

    def __len__(self):
        return len(self._tags)

    def __contains__(self, item):
        return self._tags.__contains__(item)

    def __repr__(self):
        return f'{type(self).__name__}({str(self._np_name)})'

    def __str__(self):
        return self.__repr__()
