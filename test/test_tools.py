"""
Unittests for foolysh.tools
"""

import math
import time

import pytest

from foolysh.tools import vec2
from foolysh.tools import aabb
from foolysh.tools import clock
from foolysh.tools import quadtree

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


def test_vector_math():
    v_a = vec2.Vec2()
    v_b = vec2.Vec2(1.0, 1.0)
    assert v_a - v_b == vec2.Vec2(-1, -1)
    assert v_a + v_b * 2 == vec2.Vec2(2, 2)
    assert v_a + v_b / 2 == vec2.Vec2(0.5, 0.5)
    # assert v_b // 3 == vec2.Vec2()
    assert 3 * v_b == vec2.Vec2(3, 3)
    assert 1 + v_a == vec2.Vec2(1, 1)
    assert 1 - v_b == vec2.Vec2()
    assert v_b.length == math.sqrt(2)
    assert v_b.magnitude == 2
    assert pytest.approx(v_b.normalized().length) == 1
    v_rot = v_b.rotated(90)
    assert pytest.approx(v_rot.x, 1)
    assert pytest.approx(v_rot.y, -1)
    v_rot = v_b.rotated(-90)
    assert pytest.approx(v_rot.x, -1)
    assert pytest.approx(v_rot.y, 1)
    assert v_a.normalize() is False
    assert v_b.dot(v_b) == 2


def test_aabb():
    b_a = aabb.AABB(0.5, 0.5, 0.5, 0.5)
    b_b = aabb.AABB(0.5, 0.5, 0.4, 0.4)
    b_c = aabb.AABB(0.55, 0.55, 0.45, .45)
    b_d = aabb.AABB(0, 0, 0.5, 0.5)
    p_a = vec2.Point2(1.0, 1.0)
    p_b = vec2.Point2(1 - 1e-7, 1 - 1e-7)
    assert b_a.inside_aabb(b_b) is True
    assert b_a.inside_aabb(b_c) is True
    assert b_a.overlap(b_c) is True
    assert b_a.inside_tup(p_a.x, p_a.y) is True
    assert b_a.overlap(b_d) is True
    assert b_a.inside_tup(p_b.x, p_b.y) is True


def test_quadtree():
    qt = quadtree.Quadtree()
    pos_a = aabb.AABB(0.0, 0.0, 1.0, 1.0)
    pos_b = aabb.AABB(-100.0, -100.0, 99.0, 99.0)
    pos_d = aabb.AABB(-0.5, -0.5, 0.5, 0.5)
    obj_a = 0
    obj_b = 1
    obj_d = 3
    assert qt.insert(obj_a, pos_a) is True
    assert qt.insert(obj_b, pos_b) is True
    assert qt.insert(obj_d, pos_d) is True
    result = qt.query(pos_d)
    assert len(result) == 3
    assert obj_d in result
    result = qt.query(aabb.AABB(-0.5, -0.5, 0.49, 0.49))
    assert len(result) == 2
    assert obj_a in result


def test_clock():
    clk = clock.Clock()
    clk.tick()
    start = time.perf_counter()
    time.sleep(0.1)
    clk.tick()
    stop = time.perf_counter()
    assert pytest.approx(clk.get_dt(), stop - start)
    time.sleep(0.1)
    clk.tick()
    stop = time.perf_counter()
    assert pytest.approx(clk.get_time(), stop - start)
