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
 *
 *
 * Basic 2D Vector implementation.
 */

#ifndef VEC2_HPP
#define VEC2_HPP

namespace foolysh {
namespace tools {
    class Vec2 {
    public:
        Vec2();
        Vec2(const double v);
        Vec2(const double x, const double y);
        Vec2(const Vec2& other);
        Vec2& operator=(const Vec2& other);

        double dot(const Vec2& other);
        bool normalize();
        Vec2 normalized();
        double magnitude();
        double length();
        void rotate(double a, bool radians = false);
        Vec2 rotated(double a, bool radians = false);
        bool almost_equal(Vec2& other, const double d = 1e-6);

        double& operator[](const int idx);
        Vec2 operator+(const Vec2& rhs);
        Vec2 operator+(const double rhs);
        Vec2& operator+=(const Vec2& rhs);
        Vec2& operator+=(const double rhs);
        void iadd(const Vec2& rhs);
        void iadd(const double rhs);
        Vec2 operator-(const Vec2& rhs);
        Vec2 operator-(const double rhs);
        Vec2& operator-=(const Vec2& rhs);
        Vec2& operator-=(const double rhs);
        void isub(const Vec2& rhs);
        void isub(const double rhs);
        Vec2 operator*(const double rhs);
        Vec2& operator*=(const double rhs);
        void imul(const double rhs);
        Vec2 operator/(const double rhs);
        Vec2 div(const double rhs);
        Vec2& operator/=(const double rhs);
        void idiv(const double rhs);
        bool operator==(const Vec2& rhs);
        bool operator!=(const Vec2& rhs);
        bool operator==(const double rhs);
        bool operator!=(const double rhs);
    private:
        double _x, _y, _magnitude, _length;
        unsigned char _state;  // 0 = dirty, 1 = have mag, 2 = have mag + len
    };

}  // namespace tools
}  // namespace foolysh

#endif
