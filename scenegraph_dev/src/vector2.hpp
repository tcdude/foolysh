/**
 * Basic 2D Vector implementation
 */

#ifndef VECTOR2_HPP
#define VECTOR2_HPP

namespace scenegraph {
    class Vector2 {
    public:
        Vector2();
        Vector2(const double v);
        Vector2(const double x, const double y);
        Vector2(const Vector2& other);
        Vector2& operator=(const Vector2& other);

        double dot(const Vector2& other);
        bool normalize();
        Vector2 normalized();
        double magnitude();
        double length();
        void rotate(double a, bool radians = false);
        Vector2 rotated(double a, bool radians = false);
        bool almost_equal(Vector2& other, const double d = 1e-6);

        double& operator[](const int idx);
        Vector2 operator+(const Vector2& rhs);
        Vector2 operator+(const double rhs);
        Vector2& operator+=(const Vector2& rhs);
        Vector2& operator+=(const double rhs);
        Vector2 operator-(const Vector2& rhs);
        Vector2 operator-(const double rhs);
        Vector2& operator-=(const Vector2& rhs);
        Vector2& operator-=(const double rhs);
        Vector2 operator*(const double rhs);
        Vector2& operator*=(const double rhs);
        Vector2 operator/(const double rhs);
        Vector2& operator/=(const double rhs);
        bool operator==(const Vector2& rhs);
        bool operator!=(const Vector2& rhs);
        bool operator==(const double rhs);
        bool operator!=(const double rhs);
    private:
        double _x, _y, _magnitude, _length;
        unsigned char _state;  // 0 = dirty, 1 = have mag, 2 = have mag + len
    };
}  // namespace scenegraph

#endif
