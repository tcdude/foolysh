#include "aabb.hpp"
#include <stdexcept>
#include <iostream>

/**
 * 
 */
scenegraph::AABB::
AABB() {
    x = 0.0;
    y = 0.0;
    hw = 1.0;
    hh = 1.0;
}

/**
 * 
 */
scenegraph::AABB::
AABB(double _x, double _y, double _hw, double _hh) {
    if (_hw < 0.0 && _hh < 0.0) {
        throw std::invalid_argument("invalid AABB: negative width or height");
    }
    x = _x;
    y = _y;
    hw = _hw;
    hh = _hh;
}

/**
 * Return whether ``aabb`` lies entirely inside this.
 */
bool scenegraph::AABB::
inside(const AABB& aabb) {
    const double l = x - hw, r = x + hw, t = y - hh, b = y + hh;
    return (l <= aabb.x - aabb.hw && r >= aabb.x + aabb.hw 
        && t <= aabb.y - aabb.hh && b >= aabb.y + aabb.hh) ? true : false;
}

/**
 * Return whether ``x, y`` lies entirely inside this.
 */
bool scenegraph::AABB::
inside(double _x, double _y) {
    const double l = x - hw, r = x + hw, t = y - hh, b = y + hh;
    return (l <= _x && r >= _x && t <= _y && b >= _y) ? true : false;
}

/**
 * Return whether ``aabb`` overlaps this.
 */
bool scenegraph::AABB::
overlap(const AABB& aabb) {
    const double l = x - hw, r = x + hw;
    const double l_o = aabb.x - aabb.hw;
    const double r_o = aabb.x + aabb.hw;
    if ((l <= l_o && r >= l_o) || (l <= r_o && r >= r_o)) {
        const double t = y - hh, b = y + hh;
        const double t_o = aabb.y - aabb.hh;
        const double b_o = aabb.y + aabb.hh;
        if ((t <= t_o && b >= t_o) || (t <= b_o && b >= b_o)) {
            return true;
        }
    }
    return false;
}

/**
 * Return AABB reflecting quadrant ``_q`` from this, split at the origin.
 */
scenegraph::AABB scenegraph::AABB::
split(Quadrant _q) {
    return split(x, y, _q);
}

/**
 * Return AABB reflecting quadrant ``_q`` from this, split at point ``_x/_y``.
 */
scenegraph::AABB scenegraph::AABB::
split(double _x, double _y, Quadrant _q) {
    if (! inside(_x, _y)) {
        throw std::invalid_argument("invalid x/y: not inside AABB");
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
}

/**
 * Returns the quadrant in which ``_x`` and ``_y`` lie. Does not check whether
 * the point lies inside the bounds of this AABB.
 */
scenegraph::Quadrant scenegraph::AABB::
find_quadrant(double _x, double _y) {
    if (_x < x) {
        return (_y < y) ? TL : BL;
    }
    else {
        return (_y < y) ? TR : BR;
    }
}