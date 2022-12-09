# distutils: language = c++
# cython: language_level=3

"""
Provides wrapper for the Animation System.
"""

from enum import Enum

from cython.operator cimport dereference as deref
from libcpp.memory cimport unique_ptr

from .scene.cppnode cimport Node as _Node
from .scene.node cimport Node
from .scene.cppnode cimport Scale as _Scale
from .tools.cppvec2 cimport Vec2 as _Vec2
from .tools.vec2 cimport Vec2

from .cppanimation cimport Interval as _Interval
from .cppanimation cimport Animation as _Animation
from .cppanimation cimport Sequence as _Sequence
from .cppanimation cimport AnimationManager as _AnimationManager
from .cppanimation cimport BlendType as _BlendType


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

cdef unique_ptr[_AnimationManager] __am
__am.reset(new _AnimationManager())
cdef dict __ivals = {}
cdef dict __anims = {}
cdef dict __seqs = {}


class BlendType(Enum):
    """Enumerations of blend types."""
    NO_BLEND = 0
    EASE_IN = 1
    EASE_OUT = 2
    EASE_IN_OUT = 3


cdef class AnimationBase:
    """
    Provides basic playback control for animations.
    """
    cdef bint _loop
    cdef int _id
    cdef Node _node

    def __cinit__(self, *args, **kwargs):
        self._loop = False

    def play(self):
        """
        Starts the Animation on the next animation cycle. A reference is stored
        to assure playback, even when the Animation goes out of scope.
        """
        if isinstance(self, Interval):
            __ivals[self._id] = self
            try:
                deref(__am).play_interval(self._id)
            except ArithmeticError as e:
                print('Could not play Interval')
                __ivals.pop(self._id)
                raise e
        elif isinstance(self, Animation):
            __anims[self._id] = self
            try:
                deref(__am).play_animation(self._id)
            except ArithmeticError as e:
                print('Could not play Animation')
                __anims.pop(self._id)
                raise e
        elif isinstance(self, Sequence):
            __seqs[self._id] = self
            try:
                deref(__am).play_sequence(self._id)
            except ArithmeticError as e:
                print('Could not play Sequence')
                __seqs.pop(self._id)
                raise e
        else:
            raise NotImplementedError

    def stop(self):
        """
        Stop the Animation and remove eventual reference to allow for the
        Animation to go out of scope.
        """
        if isinstance(self, Interval):
            if self._id in __ivals:
                __ivals.pop(self._id)
            try:
                deref(__am).stop_interval(self._id)
            except ArithmeticError as e:
                print('Could not stop Interval')
                raise e
        elif isinstance(self, Animation):
            if self._id in __anims:
                __anims.pop(self._id)
            try:
                deref(__am).stop_animation(self._id)
            except ArithmeticError as e:
                print('Could not stop Animation')
                raise e
        elif isinstance(self, Sequence):
            if self._id in __seqs:
                __seqs.pop(self._id)
            try:
                deref(__am).stop_sequence(self._id)
            except ArithmeticError as e:
                print('Could not stop Sequence')
                raise e
        else:
            raise NotImplementedError

    def pause(self):
        """Pause the Animation."""
        if isinstance(self, Interval):
            try:
                deref(__am).pause_interval(self._id)
            except ArithmeticError as e:
                print('Could not pause Interval')
                raise e
            except RuntimeError as e:
                print(f'An exception prevented pausing the Interval: "{e}"')
        elif isinstance(self, Animation):
            try:
                deref(__am).pause_animation(self._id)
            except ArithmeticError as e:
                print('Could not pause Animation')
                raise e
            except RuntimeError as e:
                print(f'An exception prevented pausing the Animation: "{e}"')
        elif isinstance(self, Sequence):
            try:
                deref(__am).pause_sequence(self._id)
            except ArithmeticError as e:
                print('Could not pause Sequence')
                raise e
            except RuntimeError as e:
                print(f'An exception prevented pausing the Sequence: "{e}"')
        else:
            raise NotImplementedError

    def resume(self):
        """Resume the Animation."""
        if isinstance(self, Interval):
            try:
                deref(__am).resume_interval(self._id)
            except ArithmeticError as e:
                print('Could not resume Interval')
                raise e
            except RuntimeError as e:
                print(f'An exception prevented resuming the Interval: "{e}"')
        elif isinstance(self, Animation):
            try:
                deref(__am).resume_animation(self._id)
            except ArithmeticError as e:
                print('Could not resume Animation')
                raise e
            except RuntimeError as e:
                print(f'An exception prevented resuming the Animation: "{e}"')
        elif isinstance(self, Sequence):
            try:
                deref(__am).resume_sequence(self._id)
            except ArithmeticError as e:
                print('Could not resume Sequence')
                raise e
            except RuntimeError as e:
                print(f'An exception prevented resuming the Sequence: "{e}"')
        else:
            raise NotImplementedError

    def status(self):
        return self._status()

    cdef char _status(self):
        if isinstance(self, Interval):
            try:
                return deref(__am).get_interval_status(self._id)
            except ArithmeticError as e:
                print('Unable to retrieve Interval status')
                raise e
        elif isinstance(self, Animation):
            try:
                return deref(__am).get_animation_status(self._id)
            except ArithmeticError as e:
                print('Unable to retrieve Animation status')
                raise e
        elif isinstance(self, Sequence):
            try:
                return deref(__am).get_sequence_status(self._id)
            except ArithmeticError as e:
                print('Unable to retrieve Sequence status')
                raise e
        else:
            raise NotImplementedError


cdef class AnimationType(AnimationBase):
    """
    Provides shared functionality for :class:`Interval` and :class:`Animation`
    """
    cdef dict _modifiers
    cdef _BlendType _blend_type

    def __cinit__(self, *args, **kwargs):
        self._modifiers = {}
        if 'blend' in kwargs:
            if not isinstance(kwargs['blend'], BlendType):
                raise TypeError
            self._blend_type = kwargs['blend'].value
        else:
            self._blend_type = _BlendType.NO_BLEND

    @property
    def blend(self):
        """
        :class:`BlendType` to be used.

        :setter:
            :class:`BlendType` -> sets the blend type being used.
        """
        return BlendType(self._blend_type)

    @blend.setter
    def blend(self, blend):
        if not isinstance(blend, BlendType):
            raise TypeError
        self._set_blend(blend.value)

    cdef void _set_blend(self, _BlendType blend):
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).set_blend(blend)
            except ArithmeticError as e:
                print('Unable to set BlendType to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).set_blend(blend)
            except ArithmeticError as e:
                print('Unable to set BlendType to Animation')
                raise e
        else:
            raise NotImplementedError

    def set_node(self, node):
        """
        Set the :class:`~foolysh.scene.node.Node` that will be manipulated
        """
        if not isinstance(node, Node):
            raise TypeError
        self._set_node(node)

    cdef void _set_node(self, Node node):
        if isinstance(self, Interval):
            deref(__am).get_interval(self._id).set_node(deref(node.thisptr))
        elif isinstance(self, Animation):
            deref(__am).get_animation(self._id).set_node(deref(node.thisptr))
        else:
            raise NotImplementedError

    def add_pos(self, v1, v2=None, rel=None):
        """
        Add position modifier to AnimationType.

        Args:
            v1: :class:`~foolysh.tools.vec2.Vec2` end position or start
                position, if `v2` is provided.
            v2: :class:`~foolysh.tools.vec2.Vec2` see `v1`.
            rel: :class:`~foolysh.scene.node.Node` optional relative Node.
        """
        v1a = isinstance(v1, Vec2)
        v2a = isinstance(v2, Vec2)
        rela = isinstance(rel, Node)
        if v1a and v2 is None and rel is None:
            self.add_pos_e(v1)
        elif v1a and v2a and rel is None:
            self.add_pos_es(v1, v2)
        elif v1a and v2 is None and rela:
            self.add_pos_er(v1, rel)
        elif v1a and v2a and rela:
            self.add_pos_esr(v1, v2, rel)
        else:
            raise TypeError('Illegal combination of arguments')
        self._modifiers['pos'] = (v1, v2, rel)

    cdef void add_pos_e(self, Vec2 v):
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_pos(deref(v.thisptr))
            except ArithmeticError as e:
                print('Unable to add position to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_pos(deref(v.thisptr))
            except ArithmeticError as e:
                print('Unable to add position to Animation')
                raise e
        else:
            raise NotImplementedError

    cdef void add_pos_es(self, Vec2 v1, Vec2 v2):
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_pos(
                    deref(v1.thisptr),
                    deref(v2.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add position to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_pos(
                    deref(v1.thisptr),
                    deref(v2.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add position to Animation')
                raise e
        else:
            raise NotImplementedError

    cdef void add_pos_er(self, Vec2 v, Node rel):
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_pos(
                    deref(v.thisptr),
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add position to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_pos(
                    deref(v.thisptr),
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add position to Animation')
                raise e
        else:
            raise NotImplementedError

    cdef void add_pos_esr(self, Vec2 v1, Vec2 v2, Node rel):
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_pos(
                    deref(v1.thisptr),
                    deref(v2.thisptr),
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add position to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_pos(
                    deref(v1.thisptr),
                    deref(v2.thisptr),
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add position to Animation')
                raise e
        else:
            raise NotImplementedError

    def add_scale(self, s1, s2=None, rel=None):
        """
        Add scale modifier to AnimationType.

        Args:
            s1: ``float``/``tuple[float, float]`` end scale or start scale, if
                `s2` is provided.
            s2: ``float``/``tuple[float, float]`` see `s1`.
            rel: :class:`~foolysh.scene.node.Node` optional relative Node.
        """
        if isinstance(s1, (int, float)):
            s1 = (s1, ) * 2
            s1a = True
        elif isinstance(s1, (tuple, list)) and len(s1) == 2:
            s1a = isinstance(s1[0], (int, float))
            s1a = s1a and isinstance(s1[1], (int, float))
        else:
            raise TypeError

        if isinstance(s2, (int, float)):
            s2 = (s2, ) * 2
            s2a = True
        elif isinstance(s2, (tuple, list)) and len(s2) == 2:
            s2a = isinstance(s2[0], (int, float))
            s2a = s2a and isinstance(s2[1], (int, float))
        elif s2 is None:
            s2a = False
        else:
            raise TypeError

        if isinstance(rel, Node):
            rela = True
        elif rel is None:
            rela = False
        else:
            raise TypeError

        if s1a and not s2a and not rela:
            self.add_scale_e(s1)
        elif s1a and s2a and not rela:
            self.add_scale_es(s1, s2)
        elif s1a and not s2a and rela:
            self.add_scale_er(s1, rel)
        elif s1a and s2a and rela:
            self.add_scale_esr(s1, s2, rel)
        else:
            raise TypeError('Illegal combination of arguments')
        self._modifiers['scale'] = (s1, s2, rel)

    cdef void add_scale_e(self, tuple s):
        cdef _Scale _s
        _s.sx = s[0]
        _s.sy = s[1]
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_scale(_s)
            except ArithmeticError as e:
                print('Unable to add scale to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_scale(_s)
            except ArithmeticError as e:
                print('Unable to add scale to Animation')
                raise e
        else:
            raise NotImplementedError

    cdef void add_scale_es(self, tuple s1, tuple s2):
        cdef _Scale _s1
        _s1.sx = s1[0]
        _s1.sy = s1[1]
        cdef _Scale _s2
        _s2.sx = s2[0]
        _s2.sy = s2[1]
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_scale(_s1, _s2)
            except ArithmeticError as e:
                print('Unable to add scale to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_scale(_s1, _s2)
            except ArithmeticError as e:
                print('Unable to add scale to Animation')
                raise e
        else:
            raise NotImplementedError

    cdef void add_scale_er(self, tuple s, Node rel):
        cdef _Scale _s
        _s.sx = s[0]
        _s.sy = s[1]
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_scale(
                    _s,
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add scale to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_scale(
                    _s,
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add scale to Animation')
                raise e
        else:
            raise NotImplementedError

    cdef void add_scale_esr(self, tuple s1, tuple s2, Node rel):
        cdef _Scale _s1
        _s1.sx = s1[0]
        _s1.sy = s1[1]
        cdef _Scale _s2
        _s2.sx = s2[0]
        _s2.sy = s2[1]
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_scale(
                    _s1,
                    _s2,
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add scale to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_scale(
                    _s1,
                    _s2,
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add scale to Animation')
                raise e
        else:
            raise NotImplementedError

    def add_rotation(self, d1, d2=None, rel=None):
        """
        Add rotation modifier to AnimationType.

        Args:
            d1: ``float`` end angle in degrees or start angle, if `d2` is
                provided.
            d2: ``float`` see `d1`.
            rel: :class:`~foolysh.scene.node.Node` optional relative Node.
        """
        if isinstance(d1, (int, float)):
            d1a = True
        else:
            raise TypeError

        if isinstance(d2, (int, float)):
            d2a = True
        elif d2 is None:
            d2a = False
        else:
            raise TypeError

        if isinstance(rel, Node):
            rela = True
        elif rel is None:
            rela = False
        else:
            raise TypeError

        if d1a and not d2a and not rela:
            self.add_rotation_e(d1)
        elif d1a and d2a and not rela:
            self.add_rotation_es(d1, d2)
        elif d1a and not d2a and rela:
            self.add_rotation_er(d1, rel)
        elif d1a and d2a and rela:
            self.add_rotation_esr(d1, d2, rel)
        else:
            raise TypeError('Illegal combination of arguments')
        self._modifiers['rotation'] = (d1, d2, rel)

    cdef void add_rotation_e(self, float d):
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_rotation(d)
            except ArithmeticError as e:
                print('Unable to add rotation to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_rotation(d)
            except ArithmeticError as e:
                print('Unable to add rotation to Animation')
                raise e
        else:
            raise NotImplementedError

    cdef void add_rotation_es(self, float d1, float d2):
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_rotation(d1, d2)
            except ArithmeticError as e:
                print('Unable to add rotation to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_rotation(d1, d2)
            except ArithmeticError as e:
                print('Unable to add rotation to Animation')
                raise e
        else:
            raise NotImplementedError

    cdef void add_rotation_er(self, float d, Node rel):
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_rotation(
                    d,
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add rotation to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_rotation(
                    d,
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add rotation to Animation')
                raise e
        else:
            raise NotImplementedError

    cdef void add_rotation_esr(self, float d1, float d2, Node rel):
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_rotation(
                    d1,
                    d2,
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add rotation to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_rotation(
                    d1,
                    d2,
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add rotation to Animation')
                raise e
        else:
            raise NotImplementedError

    def add_rotation_center(self, v1, v2=None):
        """
        Add rotation center position modifier to AnimationType.

        Args:
            v1: :class:`~foolysh.tools.vec2.Vec2` end position or start
                position, if `v2` is provided.
            v2: :class:`~foolysh.tools.vec2.Vec2` see `v1`.
        """
        if isinstance(v1, Vec2):
            v1a = True
        else:
            raise TypeError

        if isinstance(v2, Vec2):
            v2a = True
        elif v2 is None:
            v2a = False
        else:
            raise TypeError

        if v1a and not v2a:
            self.add_rot_center_e(v1)
        elif v1a and v2a:
            self.add_rot_center_es(v1, v2)
        else:
            raise TypeError('Illegal combination of arguments')
        self._modifiers['rotation_center'] = (v1, v2)

    cdef void add_rot_center_e(self, Vec2 v):
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_rotation_center(
                    deref(v.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add rotation center to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_rotation_center(
                    deref(v.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add rotation center to Animation')
                raise e
        else:
            raise NotImplementedError

    cdef void add_rot_center_es(self, Vec2 v1, Vec2 v2):
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_rotation_center(
                    deref(v1.thisptr),
                    deref(v2.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add rotation center to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_rotation_center(
                    deref(v1.thisptr),
                    deref(v2.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add rotation center to Animation')
                raise e
        else:
            raise NotImplementedError

    def add_depth(self, d1, d2=None, rel=None):
        """
        Add depth modifier to AnimationType.

        Args:
            d1: ``int`` end depth or start depth, if `d2` is provided.
            d2: ``int`` see `d1`.
            rel: :class:`~foolysh.scene.node.Node` optional relative Node.
        """
        if isinstance(d1, int):
            d1a = True
        else:
            raise TypeError

        if isinstance(d2, int):
            d2a = True
        elif d2 is None:
            d2a = False
        else:
            raise TypeError

        if isinstance(rel, Node):
            rela = True
        elif rel is None:
            rela = False
        else:
            raise TypeError

        if d1a and not d2a and not rela:
            self.add_depth_e(d1)
        elif d1a and d2a and not rela:
            self.add_depth_es(d1, d2)
        elif d1a and not d2a and rela:
            self.add_depth_er(d1, rel)
        elif d1a and d2a and rela:
            self.add_depth_esr(d1, d2, rel)
        else:
            raise TypeError('Illegal combination of arguments')
        self._modifiers['depth'] = (d1, d2, rel)

    cdef void add_depth_e(self, int d):
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_depth(d)
            except ArithmeticError as e:
                print('Unable to add depth to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_depth(d)
            except ArithmeticError as e:
                print('Unable to add depth to Animation')
                raise e
        else:
            raise NotImplementedError

    cdef void add_depth_es(self, int d1, int d2):
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_depth(d1, d2)
            except ArithmeticError as e:
                print('Unable to add depth to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_depth(d1, d2)
            except ArithmeticError as e:
                print('Unable to add depth to Animation')
                raise e
        else:
            raise NotImplementedError

    cdef void add_depth_er(self, int d, Node rel):
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_depth(
                    d,
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add depth to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_depth(
                    d,
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add depth to Animation')
                raise e
        else:
            raise NotImplementedError

    cdef void add_depth_esr(self, int d1, int d2, Node rel):
        if isinstance(self, Interval):
            try:
                deref(__am).get_interval(self._id).add_depth(
                    d1,
                    d2,
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add depth to Interval')
                raise e
        elif isinstance(self, Animation):
            try:
                deref(__am).get_animation(self._id).add_depth(
                    d1,
                    d2,
                    deref(rel.thisptr)
                )
            except ArithmeticError as e:
                print('Unable to add depth to Animation')
                raise e
        else:
            raise NotImplementedError


cdef class Interval(AnimationType):
    """
    Interval that allows for fixed time Node manipulation.
    Provides overloaded ``__iadd__`` operator for easy combination of several
    different Intervals to be executed in parallel.

    .. note::
        When combining Intervals, the duration of the left hand side will be
        used in the combined Interval, while ignoring the potentially different
        duration from the right hand side Interval.
    """
    cdef float _duration

    def __cinit__(self, node, *args, **kwargs):
        self._id = deref(__am).new_interval()
        self.set_node(node)
        self._node = node
        self._duration = 0.0

    def __dealloc__(self):
        deref(__am).remove_interval(self._id)

    def set_duration(self, d):
        if not isinstance(d, (int, float)):
            raise TypeError
        self._duration = d
        deref(__am).get_interval(self._id).set_duration(d)

    def __iadd__(self, other):
        if not isinstance(other, type(self)):
            raise TypeError
        self._iadd(other)
        return self

    cdef void _iadd(self, Interval other):
        if other._node.node_id != self._node.node_id:
            raise ValueError('Not the same Node')
        if 'pos' in other._modifiers:
            self.add_pos(*other._modifiers['pos'])
        if 'scale' in other._modifiers:
            self.add_scale(*other._modifiers['scale'])
        if 'rotation' in other._modifiers:
            self.add_rotation(*other._modifiers['rotation'])
        if 'rotation_center' in other._modifiers:
            self.add_rotation_center(*other._modifiers['rotation_center'])
        if 'depth' in other._modifiers:
            self.add_depth(*other._modifiers['depth'])


cdef class Animation(AnimationType):
    """
    Animation that allows for speed based Node manipulation.
    Provides overloaded ``__iadd__`` operator for easy combination of several
    different Animations to be executed in parallel.

    .. note::
        Since animations are speed based, parallel animations might finish at
        different times during execution. This might matter, when using combined
        animations in a :class:`~foolysh.animation.Sequence`.
    """
    cdef float _pos_speed
    cdef float _scale_speed
    cdef float _rotation_speed
    cdef float _rotation_center_speed
    cdef float _depth_speed

    def __cinit__(self, node, *args, **kwargs):
        self._pos_speed = 0.0
        self._scale_speed = 0.0
        self._rotation_speed = 0.0
        self._rotation_center_speed = 0.0
        self._depth_speed = 0.0
        self._id = deref(__am).new_animation()
        self.set_node(node)
        self._node = node

    def __dealloc__(self):
        deref(__am).remove_animation(self._id)

    def set_pos_speed(self, s):
        if not isinstance(s, (int, float)):
            raise TypeError
        if s <= 0:
            raise ValueError('Expected positive, non zero value')
        deref(__am).get_animation(self._id).set_pos_speed(s)
        self._pos_speed = s

    def set_scale_speed(self, s):
        if not isinstance(s, (int, float)):
            raise TypeError
        if s <= 0:
            raise ValueError('Expected positive, non zero value')
        deref(__am).get_animation(self._id).set_scale_speed(s)
        self._scale_speed = s

    def set_rotation_speed(self, s):
        if not isinstance(s, (int, float)):
            raise TypeError
        if s <= 0:
            raise ValueError('Expected positive, non zero value')
        deref(__am).get_animation(self._id).set_rotation_speed(s)
        self._rotation_speed = s

    def set_rotation_center_speed(self, s):
        if not isinstance(s, (int, float)):
            raise TypeError
        if s <= 0:
            raise ValueError('Expected positive, non zero value')
        deref(__am).get_animation(self._id).set_rotation_center_speed(s)
        self._rotation_center_speed = s

    def set_depth_speed(self, s):
        if not isinstance(s, (int, float)):
            raise TypeError
        if s <= 0:
            raise ValueError('Expected positive, non zero value')
        deref(__am).get_animation(self._id).set_depth_speed(s)
        self._depth_speed = s

    def __iadd__(self, other):
        if not isinstance(other, type(self)):
            raise TypeError
        self._iadd(other)
        return self

    cdef void _iadd(self, Animation other):
        if other._node.node_id != self._node.node_id:
            raise ValueError('Not the same Node')
        if 'pos' in other._modifiers:
            self.add_pos(*other._modifiers['pos'])
            self._pos_speed = other._pos_speed
        if 'scale' in other._modifiers:
            self.add_scale(*other._modifiers['scale'])
            self._scale_speed = other._scale_speed
        if 'rotation' in other._modifiers:
            self.add_rotation(*other._modifiers['rotation'])
            self._rotation_speed = other._rotation_speed
        if 'rotation_center' in other._modifiers:
            self.add_rotation_center(*other._modifiers['rotation_center'])
            self._rotation_center_speed = other._rotation_center_speed
        if 'depth' in other._modifiers:
            self.add_depth(*other._modifiers['depth'])
            self._depth_speed = other._depth_speed


cdef class Sequence(AnimationBase):
    """
    Sequence that allows for playing multiple instances of descendants of
    :class:`AnimationBase` in sequence.
    Provides the ``__iadd__`` operator with the same functionality as the
    :meth:`Sequence.append` method.
    """
    def __cinit__(self, *args, **kwargs):
        self._id = deref(__am).new_sequence()
        for s in args:
            if isinstance(s, AnimationBase):
                self.append(s)

    def __dealloc__(self):
        deref(__am).remove_sequence(self._id)

    @property
    def loop(self):
        """
        Property to define whether a :class:`Sequence` should be looped when
        playback reaches the end.

        :setter:
            ``bool`` -> ``True`` = loops forever / until manually stopped.
        """
        return self._loop

    @loop.setter
    def loop(self, loop):
        if not isinstance(loop, bool):
            raise TypeError
        self._loop = loop
        self._set_loop(loop)

    cdef void _set_loop(self, bint l):
        try:
            deref(__am).get_sequence(self._id).loop(l)
        except ArithmeticError as e:
            print('Unable to set loop to Sequence')
            raise e

    def append(self, item):
        """
        Append a :class:`AnimationBase` instance to this sequence.
        """
        if isinstance(item, Interval):
            self._append_interval(item)
        elif isinstance(item, Animation):
            self._append_animation(item)
        elif isinstance(item, Sequence):
            self._append_sequence(item)
        else:
            raise TypeError

    cdef void _append_interval(self, Interval item):
        deref(__am).append(self._id, item._id)

    cdef void _append_animation(self, Animation item):
        deref(__am).append(self._id, item._id)

    cdef void _append_sequence(self, Sequence item):
        deref(__am).append(self._id, item._id)

    def __iadd__(self, other):
        self.append(other)
        return self


def PosInterval(node, duration, v1, v2=None, rel=None, blend=None):
    """
    Factory method to create an :class:`Interval` instance with position
    modifier.

    Args:
        node: :class:`~foolysh.scene.node.Node` the manipulated node instance.
        duration: ``float`` duration of the Interval in seconds.
        v1: :class:`~foolysh.tools.vec2.Vec2` the end or start position,
            depending on whether `v2` is provided.
        v2: see `v1`
        rel: :class:`~foolysh.scene.node.Node` optional relative node.
        blend: :class:`~foolysh.animation.BlendType` optional blending.
    """
    if blend is None:
        ival = Interval(node)
    else:
        ival = Interval(node, blend=blend)
    ival.set_duration(duration)
    ival.add_pos(v1, v2, rel)
    return ival


def ScaleInterval(node, duration, s1, s2=None, rel=None, blend=None):
    """
    Factory method to create an :class:`Interval` instance with scale modifier.

    Args:
        node: :class:`~foolysh.scene.node.Node` the manipulated node instance.
        duration: ``float`` duration of the Interval in seconds.
        s1: ``float``/``tuple`` the end or start scale, depending on whether
            `s2` is provided.
        s2: see `s1`
        rel: :class:`~foolysh.scene.node.Node` optional relative node.
        blend: :class:`~foolysh.animation.BlendType` optional blending.
    """
    if blend is None:
        ival = Interval(node)
    else:
        ival = Interval(node, blend=blend)
    ival.set_duration(duration)
    ival.add_scale(s1, s2, rel)
    return ival


def RotationInterval(node, duration, d1, d2=None, rel=None, blend=None):
    """
    Factory method to create an :class:`Interval` instance with rotation
    modifier.

    Args:
        node: :class:`~foolysh.scene.node.Node` the manipulated node instance.
        duration: ``float`` duration of the Interval in seconds.
        d1: ``float`` the end or start angle in degrees, depending on whether
            `d2` is provided.
        d2: see `d1`
        rel: :class:`~foolysh.scene.node.Node` optional relative node.
        blend: :class:`~foolysh.animation.BlendType` optional blending.
    """
    if blend is None:
        ival = Interval(node)
    else:
        ival = Interval(node, blend=blend)
    ival.set_duration(duration)
    ival.add_rotation(d1, d2, rel)
    return ival


def RotationCenterInterval(node, duration, v1, v2=None, blend=None):
    """
    Factory method to create an :class:`Interval` instance with rotation center
    modifier.

    Args:
        node: :class:`~foolysh.scene.node.Node` the manipulated node instance.
        duration: ``float`` duration of the Interval in seconds.
        v1: :class:`~foolysh.tools.vec2.Vec2` the end or start position of
            the rotation center, depending on whether `v2` is provided.
        v2: see `v1`
        blend: :class:`~foolysh.animation.BlendType` optional blending.
    """
    if blend is None:
        ival = Interval(node)
    else:
        ival = Interval(node, blend=blend)
    ival.set_duration(duration)
    ival.add_rotation_center(v1, v2)
    return ival


def DepthInterval(node, duration, d1, d2=None, rel=None, blend=None):
    """
    Factory method to create an :class:`Interval` instance with depth modifier.

    Args:
        node: :class:`~foolysh.scene.node.Node` the manipulated node instance.
        duration: ``float`` duration of the Interval in seconds.
        d1: ``int`` the end or start depth, depending on whether `d2` is
            provided.
        d2: see `d1`
        rel: :class:`~foolysh.scene.node.Node` optional relative node.
        blend: :class:`~foolysh.animation.BlendType` optional blending.
    """
    if blend is None:
        ival = Interval(node)
    else:
        ival = Interval(node, blend=blend)

    ival.set_duration(duration)
    ival.add_depth(d1, d2, rel)
    return ival


def PosAnimation(node, speed, v1, v2=None, rel=None, blend=None):
    """
    Factory method to create an :class:`Animation` instance with position
    modifier.

    Args:
        node: :class:`~foolysh.scene.node.Node` the manipulated node instance.
        speed: ``float`` speed of the Animation in units per second.
        v1: :class:`~foolysh.tools.vec2.Vec2` the end or start position,
            depending on whether `v2` is provided.
        v2: see `v1`
        rel: :class:`~foolysh.scene.node.Node` optional relative node.
        blend: :class:`~foolysh.animation.BlendType` optional blending.
    """
    if blend is None:
        anim = Animation(node)
    else:
        anim = Animation(node, blend=blend)
    anim.set_pos_speed(speed)
    anim.add_pos(v1, v2, rel)
    return anim


def ScaleAnimation(node, speed, s1, s2=None, rel=None, blend=None):
    """
    Factory method to create an :class:`Animation` instance with scale modifier.

    Args:
        node: :class:`~foolysh.scene.node.Node` the manipulated node instance.
        speed: ``float`` speed of the Animation in units per second.
        s1: ``float``/``tuple`` the end or start scale, depending on whether
            `s2` is provided.
        s2: see `s1`
        rel: :class:`~foolysh.scene.node.Node` optional relative node.
        blend: :class:`~foolysh.animation.BlendType` optional blending.
    """
    if blend is None:
        anim = Animation(node)
    else:
        anim = Animation(node, blend=blend)
    anim.set_scale_speed(speed)
    anim.add_scale(s1, s2, rel)
    return anim


def RotationAnimation(node, speed, d1, d2=None, rel=None, blend=None):
    """
    Factory method to create an :class:`Animation` instance with rotation
    modifier.

    Args:
        node: :class:`~foolysh.scene.node.Node` the manipulated node instance.
        speed: ``float`` speed of the Animation in units per second.
        d1: ``float`` the end or start angle in degrees, depending on whether
            `d2` is provided.
        d2: see `d1`
        rel: :class:`~foolysh.scene.node.Node` optional relative node.
        blend: :class:`~foolysh.animation.BlendType` optional blending.
    """
    if blend is None:
        anim = Animation(node)
    else:
        anim = Animation(node, blend=blend)
    anim.set_rotation_speed(speed)
    anim.add_rotation(d1, d2, rel)
    return anim


def RotationCenterAnimation(node, speed, v1, v2=None, blend=None):
    """
    Factory method to create an :class:`Animation` instance with rotation center
    modifier.

    Args:
        node: :class:`~foolysh.scene.node.Node` the manipulated node instance.
        speed: ``float`` speed of the Animation in units per second.
        v1: :class:`~foolysh.tools.vec2.Vec2` the end or start position of
            the rotation center, depending on whether `v2` is provided.
        v2: see `v1`
        blend: :class:`~foolysh.animation.BlendType` optional blending.
    """
    if blend is None:
        anim = Animation(node)
    else:
        anim = Animation(node, blend=blend)
    anim.set_rotation_center_speed(speed)
    anim.add_rotation_center(v1, v2)
    return anim


def DepthAnimation(node, speed, d1, d2=None, rel=None, blend=None):
    """
    Factory method to create an :class:`Animation` instance with depth modifier.

    Args:
        node: :class:`~foolysh.scene.node.Node` the manipulated node instance.
        speed: ``float`` speed of the Animation in units per second.
        d1: ``int`` the end or start depth, depending on whether `d2` is
            provided.
        d2: see `d1`
        rel: :class:`~foolysh.scene.node.Node` optional relative node.
        blend: :class:`~foolysh.animation.BlendType` optional blending.
    """
    if blend is None:
        anim = Animation(node)
    else:
        anim = Animation(node, blend=blend)
    anim.set_depth_speed(speed)
    anim.add_depth(d1, d2, rel)
    return anim


cdef class AnimationManager:
    """
    Provides control when to update all currently active :class:`AnimationBase`
    instances (:class:`Interval`, :class:`Animation`, :class:`Sequence`).

    .. note::
        Normally there is no need to use this class, as the functionality is
        already provided when subclassing :class:`~foolysh.app.App` in your
        application and :meth:`AnimationManager.animate` is called each frame
        automatically.
    """
    def animate(self, dt, **kwargs):
        self._animate(dt)

    cdef void _animate(self, const double dt):
        """Advance active animations by `dt` seconds."""
        deref(__am).animate(dt)
        self.clean_up()

    cdef void clean_up(self):
        cdef char c
        cdef list r = []
        for d in (__ivals, __anims, __seqs):
            r = []
            for k in d:
                c = d[k].status()
                if c == 0 or c == 4:
                    r.append(k)
            for k in r:
                d.pop(k)
