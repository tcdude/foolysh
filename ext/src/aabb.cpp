/**
 * Copyright (c) 2020 Tiziano Bettio
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "aabb.hpp"
// #include <stdexcept>
#include <iostream>

namespace foolysh {
namespace tools {

/**
 *
 */
AABB::
AABB() {
    x = 0.0;
    y = 0.0;
    hw = 1.0;
    hh = 1.0;
}

/**
 *
 */
AABB::
AABB(double _x, double _y, double _hw, double _hh) {
    if (_hw < 0.0 && _hh < 0.0) {
        // throw std::invalid_argument("invalid AABB: negative width or height");
    }
    x = _x;
    y = _y;
    hw = _hw;
    hh = _hh;
}

/**
 * Return whether ``aabb`` lies entirely inside this.
 */
bool AABB::
inside(const AABB& aabb) {
    const double l = x - hw, r = x + hw, t = y - hh, b = y + hh;
    return (l <= aabb.x - aabb.hw && r >= aabb.x + aabb.hw
        && t <= aabb.y - aabb.hh && b >= aabb.y + aabb.hh) ? true : false;
}

/**
 * Return whether ``x, y`` lies entirely inside this.
 */
bool AABB::
inside(double _x, double _y) {
    const double l = x - hw, r = x + hw, t = y - hh, b = y + hh;
    return (l <= _x && r >= _x && t <= _y && b >= _y) ? true : false;
}

/**
 * Return whether ``aabb`` overlaps this.
 */
bool AABB::
overlap(const AABB& aabb) {
    const double l = x - hw, r = x + hw;
    const double l_o = aabb.x - aabb.hw;
    const double r_o = aabb.x + aabb.hw;
    if ((l <= l_o && r >= l_o) || (l <= r_o && r >= r_o)
        || (l_o <= l && r_o >= l) || (l_o <= r && r_o >= r)) {
        const double t = y - hh, b = y + hh;
        const double t_o = aabb.y - aabb.hh;
        const double b_o = aabb.y + aabb.hh;
        if ((t <= t_o && b >= t_o) || (t <= b_o && b >= b_o)
            || (t_o <= t && b_o >= t) || (t_o <= b && b_o >= b)) {
            return true;
        }
    }
    return false;
}

/**
 * Return AABB reflecting quadrant ``_q`` from this, split at the origin.
 */
AABB AABB::
split(Quadrant _q) {
    return split(x, y, _q);
}

/**
 * Return AABB reflecting quadrant ``_q`` from this, split at point ``_x/_y``.
 */
AABB AABB::
split(double _x, double _y, Quadrant _q) {
    if (! inside(_x, _y)) {
        // throw std::invalid_argument("invalid x/y: not inside AABB");
        return AABB();
    }
    double cx, cy, w, h;
    switch (_q) {
        case TL: {
            w = (_x - (x - hw)) / 2.0;
            h = (_y - (y - hw)) / 2.0;
            cx = _x - w;
            cy = _y - h;
            return AABB(cx, cy, w, h);
        }
        case TR: {
            w = ((x + hw) - _x) / 2.0;
            h = (_y - (y - hw)) / 2.0;
            cx = _x + w;
            cy = _y - h;
            return AABB(cx, cy, w, h);
        }
        case BL: {
            w = (_x - (x - hw)) / 2.0;
            h = ((y + hw) - _y) / 2.0;
            cx = _x - w;
            cy = _y + h;
            return AABB(cx, cy, w, h);
        }
        case BR: {
            w = ((x + hw) - _x) / 2.0;
            h = ((y + hw) - _y) / 2.0;
            cx = _x + w;
            cy = _y + h;
            return AABB(cx, cy, w, h);
        }
    }
    // throw std::range_error("Invalid Quadrant");
    return AABB();
}

/**
 * Returns the quadrant in which ``_x`` and ``_y`` lie. Does not check whether
 * the point lies inside the bounds of this AABB.
 */
Quadrant AABB::
find_quadrant(double _x, double _y) {
    if (_x < x) {
        return (_y < y) ? TL : BL;
    }
    else {
        return (_y < y) ? TR : BR;
    }
}

}  // namespace tools
}  // namespace foolysh
