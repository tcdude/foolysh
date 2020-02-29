"""
Unittests for foolysh.scene
"""
# noinspection PyPackageRequirements
import pytest

from foolysh.scene import layout
from foolysh.scene import node
from foolysh import tools
from foolysh.tools import aabb

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


def create_empty_nd():
    """Setup function to create an empty Node."""
    nd = node.Node()
    nd.size = 1.0, 1.0
    nd.traverse()
    return nd


def test_node_relative_pos():
    """Verify relative positioning."""
    nd = create_empty_nd()
    assert nd.relative_pos == tools.vec2.Point2()
    nd.origin = tools.common.Origin.BOTTOM_RIGHT
    assert nd.traverse() is True
    assert nd.relative_pos == tools.vec2.Point2(-1.0, -1.0)
    nd.origin = tools.common.Origin.CENTER
    assert nd.traverse() is True
    assert nd.relative_pos == tools.vec2.Point2(-0.5, -0.5)
    nd.remove()


def test_node_nesting():
    """Verify nesting of nodes."""
    nd = create_empty_nd()
    child = nd
    for _ in range(1000):
        child = child.attach_node()
        child.pos = 0.1, 0.1
        child.size = 1.0, 1.0
    assert nd.traverse() is True
    assert pytest.approx(child.relative_pos.x, 100.0)
    assert pytest.approx(child.relative_pos.y, 100.0)
    nd.angle = 90
    assert nd.traverse() is True
    assert pytest.approx(child.relative_pos.x, 100.0)
    assert pytest.approx(child.relative_pos.y, -100.0)


def test_node_query():
    """Verify querying of nodes."""
    nd = create_empty_nd()
    child = nd.attach_node()
    child.size = 0.25, 0.25
    child.pos = 0.8, 0.8
    assert nd.traverse() is True
    assert len(nd.query(aabb.AABB(0.5, 0.5, 0.5, 0.5))) == 2
    result = nd.query(aabb.AABB(1.055, 1.055, 0.045, 0.045))
    assert len(result) == 1
    assert child in result
    result = nd.query(aabb.AABB(0.1, 0.1, 0.1, 0.1))
    assert len(result) == 1
    assert nd in result
    nd.remove()


def test_node_dirty():
    """Verify dirty propagation."""
    nd = create_empty_nd()
    c = nd.attach_node()
    c.size = 0.25, 0.25
    c.pos = 0.8, 0.8
    assert nd.traverse() is True
    assert nd.traverse() is False
    nd.remove()


def test_grid_layout():
    """Verify GridLayout."""
    nd = create_empty_nd()
    grid = layout.GridLayout(
        nd,
        (0.0, 0.0, 1.0, 1.5),
        rows=[5],
        cols=[10, 20, None, 20, None]
    )
    assert nd.traverse() is True
    assert grid[1, 1].size == pytest.approx((0.2, 0.3))
    assert grid[1, 2].size == pytest.approx((0.25, 0.3))
    assert grid[1, 1].relative_pos == pytest.approx((0.1, 0.3))
    assert grid[1, 2].relative_pos == pytest.approx((0.3, 0.3))
    assert grid[3, 3].relative_pos == pytest.approx((0.55, 0.9))


def test_grid_layout_margins():
    """Verify GridLayout margins."""
    nd = create_empty_nd()
    with pytest.raises(ValueError) as e_info:
        _ = layout.GridLayout(
            nd,
            (0.0, 0.0, 1.0, 1.5),
            rows=[5],
            cols=[10, 20, None, 20, None],
            margins=(0.05, 0.05)
        )
    assert e_info.match(r'.*margins are equal or larger.*')
    nd = create_empty_nd()
    grid = layout.GridLayout(
        nd,
        (0.0, 0.0, 1.0, 1.5),
        rows=[5],
        cols=[10, 20, None, 20, None],
        margins=(0.01, 0.01)
    )
    assert nd.traverse() is True
    assert grid[1, 1].size == pytest.approx((0.18, 0.28))
    assert grid[1, 2].size == pytest.approx((0.23, 0.28))
    assert grid[1, 1].relative_pos == pytest.approx((0.11, 0.31))
    assert grid[1, 2].relative_pos == pytest.approx((0.31, 0.31))
    assert grid[3, 3].relative_pos == pytest.approx((0.56, 0.91))
