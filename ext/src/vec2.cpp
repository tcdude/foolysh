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

/**
 * Basic 2D Vector implementation
 */

#include "vec2.hpp"

#include <iostream>
#include <cmath>
#include <stdexcept>

#include "common.hpp"

namespace foolysh {
namespace tools {

/**
 * Default c-tor, initialize vector to x = y = 0.0
 */
Vec2::
Vec2() {
    _x = _y = _magnitude = _length = 0.0;
    _state = 0;
}

/**
 * Initialize vector to x = y = v
 */
Vec2::
Vec2(const double v) {
    _x = _y = v;
    _magnitude = _length = 0.0;
    _state = 0;
}

/**
 * Initialize with x and y
 */
Vec2::
Vec2(const double x, const double y) {
    _x = x;
    _y = y;
    _magnitude = _length = 0.0;
    _state = 0;
}

/**
 * Copy c-tor
 */
Vec2::
Vec2(const Vec2& other) : _x(other._x), _y(other._y),
                                _magnitude(other._magnitude),
                                _length(other._length), _state(other._state) {
}

/**
 * Assignment c-tor
 */
Vec2& Vec2::
operator=(const Vec2& other) {
    _x = other._x;
    _y = other._y;
    _magnitude = other._magnitude;
    _length = other._length;
    _state = other._state;
    return *this;
}

/**
 * Dot product
 */
double Vec2::
dot(const Vec2& other) {
    return _x * other._x + _y * other._y;
}

/**
 * Return true if length > 0 otherwise false
 */
bool Vec2::
normalize() {
    double l = length();
    if (l > 0.0) {
        _x /= l;
        _y /= l;
        _length = _magnitude = 1.0;
        _state = 2;
        return true;
    }
    return false;
}

/**
 * Return normalized Vec2 of this
 */
Vec2 Vec2::
normalized() {
    Vec2 v = Vec2(*this);
    if (v.normalize()) {
        std::cout << "OK" << std::endl;
        return v;
    }
    throw std::underflow_error("Cannot normalize Vec2 of zero length.");
}

/**
 * Return the magnitude of Vec2
 */
double Vec2::
magnitude() {
    if (_state < 1) {
        _magnitude = dot(*this);
        _state = 1;
    }
    return _magnitude;
}

/**
 * Return the length of Vec2
 */
double Vec2::
length() {
    if (_state < 2) {
        const double mag = magnitude();
        _length = (mag > 0) ? std::sqrt(mag) : 0.0;
        _state = 2;
    }
    return _length;
}

/**
 * Rotates this Vec2 by ``a`` degrees/radians in a clockwise direction
 * depending on argument ``radians``. Default is false (=degrees).
 */
void Vec2::
rotate(double a, bool radians) {
    if (!radians) {
        a *= -to_rad;
    }
    const double sa = std::sin(a), ca = std::cos(a);
    const double x = ca * _x - sa * _y;
    const double y = sa * _x + ca * _y;
    _x = x;
    _y = y;
    _state = 0;
}

/**
 * Return a Vec2 of this instance, rotated by ``a`` degrees/radians in a
 * clockwise direction depending on argument ``radians``. Default is
 * false (=degrees).
 */
Vec2 Vec2::
rotated(double a, bool radians) {
    if (!radians) {
        a *= -to_rad;
    }
    const double sa = std::sin(a), ca = std::cos(a);
    return Vec2(ca * _x - sa * _y, sa * _x + ca * _y);
}

/**
 * Return true if almost equal. Use ``d`` for allowed delta.
 */
bool Vec2::
almost_equal(Vec2& other, const double d) {
    return (std::fabs(_x - other._x) + std::fabs(_y - other._y) <= d) ? true : false;
}

/**
 *
 */
double& Vec2::
operator[](const int idx) {
    if (idx == 0 || idx == 1) {
        _state = 0;
        return (idx == 0) ? _x : _y;
    }
    throw std::range_error("Index out of range");
}

/**
 * this + Vec2
 */
Vec2 Vec2::
operator+(const Vec2& rhs) {
    return Vec2(_x + rhs._x, _y + rhs._y);
}

/**
 * this + double
 */
Vec2 Vec2::
operator+(const double rhs) {
    return Vec2(_x + rhs, _y + rhs);
}

/**
 * this += Vec2
 */
Vec2& Vec2::
operator+=(const Vec2& rhs) {
    _state = 0;
    _x += rhs._x;
    _y += rhs._y;
    return *this;
}

/**
 * this += double
 */
Vec2& Vec2::
operator+=(const double rhs) {
    _state = 0;
    _x += rhs;
    _y += rhs;
    return *this;
}

/**
 * this += Vec2
 */
void Vec2::
iadd(const Vec2& rhs) {
    *this += rhs;
}

/**
 * this += double
 */
void Vec2::
iadd(const double rhs) {
    *this += rhs;
}

/**
 * this - Vec2
 */
Vec2 Vec2::
operator-(const Vec2& rhs) {
    return Vec2(_x - rhs._x, _y - rhs._y);
}

/**
 * this - double
 */
Vec2 Vec2::
operator-(const double rhs) {
    return Vec2(_x - rhs, _y - rhs);
}

/**
 * this -= Vec2
 */
Vec2& Vec2::
operator-=(const Vec2& rhs) {
    _state = 0;
    _x -= rhs._x;
    _y -= rhs._y;
    return *this;
}

/**
 * this -= double
 */
Vec2& Vec2::
operator-=(const double rhs) {
    _state = 0;
    _x -= rhs;
    _y -= rhs;
    return *this;
}

/**
 * this -= Vec2
 */
void Vec2::
isub(const Vec2& rhs) {
    *this -= rhs;
}

/**
 * this -= double
 */
void Vec2::
isub(const double rhs) {
    *this -= rhs;
}

/**
 * this * double
 */
Vec2 Vec2::
operator*(const double rhs) {
    return Vec2(_x * rhs, _y * rhs);
}

/**
 * this *= double
 */
Vec2& Vec2::
operator*=(const double rhs) {
    _state = 0;
    _x *= rhs;
    _y *= rhs;
    return *this;
}

/**
 * this *= double
 */
void Vec2::
imul(const double rhs) {
    *this *= rhs;
}

/**
 * this / double
 */
Vec2 Vec2::
operator/(const double rhs) {
    if (rhs) {
        return Vec2(_x / rhs, _y / rhs);
    }
    throw std::underflow_error("Division by zero.");
}

/**
 * Ugly hack because cython cannot handle "operator/" atm.
 */
Vec2 Vec2::
div(const double rhs) {
    return *this / rhs;
}

/**
 * this /= double
 */
Vec2& Vec2::
operator/=(const double rhs) {
    if (rhs) {
        _state = 0;
        _x /= rhs;
        _y /= rhs;
        return *this;
    }
    throw std::underflow_error("Division by zero.");
}

/**
 * this /= double
 */
void Vec2::
idiv(const double rhs) {
    *this /= rhs;
}

/**
 *
 */
bool Vec2::
operator==(const Vec2& rhs) {
    return (_x == rhs._x && _y == rhs._y);
}

/**
 *
 */
bool Vec2::
operator!=(const Vec2& rhs) {
    return !(*this == rhs);
}

/**
 *
 */
bool Vec2::
operator==(const double rhs) {
    return (_x == rhs && _y == rhs);
}

/**
 *
 */
bool Vec2::
operator!=(const double rhs) {
    return !(*this == rhs);
}


}  // namespace tools
}  // namespace foolysh
