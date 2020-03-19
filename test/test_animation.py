"""
Unittests for foolysh.animation
"""

from foolysh import animation
from foolysh.scene import node
from foolysh.tools import vec2

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


def test_pos_interval():
    nd = node.Node()
    b = vec2.Vec2(0)
    e = vec2.Vec2(1)
    animation.PosInterval(nd, 1.0, b, e).play()
    aam = animation.AnimationManager()
    assert nd.traverse() is True
    assert nd.pos == b
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.pos == vec2.Vec2(0.5)
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.pos == e
    nd.remove()


def test_scale_interval():
    nd = node.Node()
    b = 1.0, 2.0
    e = 2.0, 1.0
    nd.scale = b
    animation.ScaleInterval(nd, 1.0, b, e).play()
    aam = animation.AnimationManager()
    assert nd.traverse() is True
    assert nd.scale == b
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.scale == (1.5, 1.5)
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.scale == e
    nd.remove()


def test_rotation_interval():
    nd = node.Node()
    b = 0.0
    e = 180.0
    animation.RotationInterval(nd, 1.0, b, e).play()
    aam = animation.AnimationManager()
    assert nd.traverse() is True
    assert nd.angle == b
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.angle == 90.0
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.angle == e
    nd.remove()


def test_rot_center_interval():
    nd = node.Node()
    b = vec2.Vec2(0)
    e = vec2.Vec2(1)
    animation.RotationCenterInterval(nd, 1.0, b, e).play()
    aam = animation.AnimationManager()
    assert nd.traverse() is True
    assert nd.rotation_center == b
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.rotation_center == vec2.Vec2(0.5)
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.rotation_center == e
    nd.remove()


def test_depth_interval():
    nd = node.Node()
    b = 1
    e = 11
    animation.DepthInterval(nd, 1.0, b, e).play()
    aam = animation.AnimationManager()
    assert nd.traverse() is True
    assert nd.depth == b
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.depth == 6
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.depth == e
    nd.remove()


def test_combined_interval():
    nd = node.Node()
    pb = vec2.Vec2(0)
    pe = vec2.Vec2(1)
    db = 1
    de = 11
    ival = animation.PosInterval(nd, 1.0, pb, pe)
    ival += animation.DepthInterval(nd, 1.0, db, de)
    ival.play()
    aam = animation.AnimationManager()
    assert nd.traverse() is True
    assert nd.pos == pb
    assert nd.depth == db
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.pos == vec2.Vec2(0.5)
    assert nd.depth == 6
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.pos == pe
    assert nd.depth == de
    nd.remove()


def test_pos_animation():
    nd = node.Node()
    b = vec2.Vec2(0)
    e = vec2.Vec2(1, 0)
    animation.PosAnimation(nd, 1.0, b, e).play()
    aam = animation.AnimationManager()
    assert nd.traverse() is True
    assert nd.pos == b
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.pos == vec2.Vec2(0.5, 0)
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.pos == e
    nd.remove()


def test_scale_animation():
    nd = node.Node()
    b = 1.0, 2.0
    e = 2.0, 1.0
    nd.scale = b
    animation.ScaleAnimation(nd, 1.0, b, e).play()
    aam = animation.AnimationManager()
    assert nd.traverse() is True
    assert nd.scale == b
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.scale == (1.5, 1.5)
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.scale == e
    nd.remove()


def test_rotation_animation():
    nd = node.Node()
    b = 0.0
    e = 180.0
    animation.RotationAnimation(nd, 180.0, b, e).play()
    aam = animation.AnimationManager()
    assert nd.traverse() is True
    assert nd.angle == b
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.angle == 90.0
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.angle == e
    nd.remove()


def test_rot_center_animation():
    nd = node.Node()
    b = vec2.Vec2(0)
    e = vec2.Vec2(1, 0)
    animation.RotationCenterAnimation(nd, 1.0, b, e).play()
    aam = animation.AnimationManager()
    assert nd.traverse() is True
    assert nd.rotation_center == b
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.rotation_center == vec2.Vec2(0.5, 0)
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.rotation_center == e
    nd.remove()


def test_depth_animation():
    nd = node.Node()
    b = 1
    e = 11
    animation.DepthAnimation(nd, 10, b, e).play()
    aam = animation.AnimationManager()
    assert nd.traverse() is True
    assert nd.depth == b
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.depth == 6
    aam.animate(0.5)
    assert nd.traverse() is True
    assert nd.depth == e
    nd.remove()
