/**
 * Basic 2D Vector implementation
 */

#include "vector2.hpp"

#include <cmath>
#include <stdexcept>

constexpr double to_deg = 180.0 / 3.14159265358979323846;
constexpr double to_rad = 3.14159265358979323846 / 180.0;


/**
 * Default c-tor, initialize vector to x = y = 0.0
 */
scenegraph::Vector2::
Vector2() {
    _x = _y = _magnitude = _length = 0.0;
    _state = 0;
}

/**
 * Initialize vector to x = y = v
 */
scenegraph::Vector2::
Vector2(const double v) {
    _x = _y = v;
    _magnitude = _length = 0.0;
    _state = 0;
}

/**
 * Initialize with x and y
 */
scenegraph::Vector2::
Vector2(const double x, const double y) {
    _x = x;
    _y = y;
    _magnitude = _length = 0.0;
    _state = 0;
}

/**
 * Copy c-tor
 */
scenegraph::Vector2::
Vector2(const Vector2& other) {
    _x = other._x;
    _y = other._y;
    _magnitude = other._magnitude;
    _length = other._length;
    _state = other._state;
}

/**
 * Assignment c-tor
 */
scenegraph::Vector2& scenegraph::Vector2::
operator=(const Vector2& other) {
    return *this = Vector2(other);
}

/**
 * Dot product
 */
double scenegraph::Vector2::
dot(const Vector2& other) {
    return _x * other._x + _y * other._y;
}

/**
 * Return true if length > 0 otherwise false
 */
bool scenegraph::Vector2::
normalize() {
    double l = length();
    if (l > 0.0) {
        _x /= l;
        _y /= l;
        return true;
    }
    return false;
}

/**
 * Return normalized Vector2 of this
 */
scenegraph::Vector2 scenegraph::Vector2::
normalized() {
    Vector2 v = Vector2(*this);
    if (v.normalize()) {
        return v;
    }
    throw std::underflow_error("Cannot normalize Vector2 of zero length.");
}

/**
 * Return the magnitude of Vector2
 */
double scenegraph::Vector2::
magnitude() {
    if (_state < 1) {
        _magnitude = dot(*this);
        _state = 1;
    }
    return _magnitude;
}

/**
 * Return the length of Vector2
 */
double scenegraph::Vector2::
length() {
    if (_state < 2) {
        const double mag = magnitude();
        _length = (mag > 0) ? std::sqrt(mag) : 0.0;
        _state = 2;
    }
    return _length;
}

/**
 * Rotates this Vector2 by ``a`` degrees if ``radians`` is false, otherwise by
 * ``a`` radians.
 */
void scenegraph::Vector2::
rotate(double a, bool radians) {
    if (!radians) {
        a *= -to_rad;
    }
    const double sa = std::sin(a), ca = std::cos(a);
    _x = ca * _x - sa * _y;
    _y = sa * _x + ca * _y;
    _state = 0;
}

/**
 * Return a rotated Vector2 by ``a`` degrees if ``radians`` is false, otherwise 
 * by ``a`` radians.
 */
scenegraph::Vector2 scenegraph::Vector2::
rotated(double a, bool radians) {
    if (!radians) {
        a *= -to_rad;
    }
    const double sa = std::sin(a), ca = std::cos(a);
    return Vector2(ca * _x - sa * _y, sa * _x + ca * _y);
}

/**
 * Return true if almost equal. Use ``d`` for allowed delta.
 */
bool scenegraph::Vector2::
almost_equal(Vector2& other, const double d) {
    return (std::fabs(_x - other._x) + std::fabs(_y - other._y) <= d) ? true : false;
}

/**
 * 
 */
double& scenegraph::Vector2::
operator[](const int idx) {
    if (-1 < idx < 2) {
        _state = 0;
        return (idx == 0) ? _x : _y;
    }
}

/**
 * this + Vector2
 */
scenegraph::Vector2 scenegraph::Vector2::
operator+(const Vector2& rhs) {
    return Vector2(_x + rhs._x, _y + rhs._y);
}

/**
 * this + double
 */
scenegraph::Vector2 scenegraph::Vector2::
operator+(const double rhs) {
    return Vector2(_x + rhs, _y + rhs);
}

/**
 * this += Vector2
 */
scenegraph::Vector2& scenegraph::Vector2::
operator+=(const Vector2& rhs) {
    _state = 0;
    _x += rhs._x;
    _y += rhs._y;
    return *this;
}

/**
 * this += double
 */
scenegraph::Vector2& scenegraph::Vector2::
operator+=(const double rhs) {
    _state = 0;
    _x += rhs;
    _y += rhs;
    return *this;
}

/**
 * this - Vector2
 */
scenegraph::Vector2 scenegraph::Vector2::
operator-(const Vector2& rhs) {
    return Vector2(_x - rhs._x, _y - rhs._y);
}

/**
 * this - double
 */
scenegraph::Vector2 scenegraph::Vector2::
operator-(const double rhs) {
    return Vector2(_x - rhs, _y - rhs);
}

/**
 * this -= Vector2
 */
scenegraph::Vector2& scenegraph::Vector2::
operator-=(const Vector2& rhs) {
    _state = 0;
    _x -= rhs._x;
    _y -= rhs._y;
    return *this;
}

/**
 * this -= double
 */
scenegraph::Vector2& scenegraph::Vector2::
operator-=(const double rhs) {
    _state = 0;
    _x -= rhs;
    _y -= rhs;
    return *this;
}

/**
 * this * double
 */
scenegraph::Vector2 scenegraph::Vector2::
operator*(const double rhs) {
    return Vector2(_x * rhs, _y * rhs);
}

/**
 * this *= double
 */
scenegraph::Vector2& scenegraph::Vector2::
operator*=(const double rhs) {
    _state = 0;
    _x *= rhs;
    _y *= rhs;
    return *this;
}

/**
 * this / double
 */
scenegraph::Vector2 scenegraph::Vector2::
operator/(const double rhs) {
    return Vector2(_x / rhs, _y / rhs);
}

/**
 * this /= double
 */
scenegraph::Vector2& scenegraph::Vector2::
operator/=(const double rhs) {
    _state = 0;
    _x /= rhs;
    _y /= rhs;
    return *this;
}

/**
 * 
 */
bool scenegraph::Vector2::
operator==(const Vector2& rhs) {
    return (_x == rhs._x && _y == rhs._y);
}

/**
 * 
 */
bool scenegraph::Vector2::
operator!=(const Vector2& rhs) {
    return !(*this == rhs);
}

/**
 * 
 */
bool scenegraph::Vector2::
operator==(const double rhs) {
    return (_x == rhs && _y == rhs);
}

/**
 * 
 */
bool scenegraph::Vector2::
operator!=(const double rhs) {
    return !(*this == rhs);
}
