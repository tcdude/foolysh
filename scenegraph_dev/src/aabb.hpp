/**
 * Copyright (c) 2019 Tiziano Bettio
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
 * Simple 2D Axis Aligned Bounding Box implementation.
 */

#ifndef AABB_HPP
#define AABB_HPP


namespace scenegraph {
    enum Quadrant {
        TL,
        TR,
        BL,
        BR
    };
    class AABB {
    public:
        AABB();
        AABB(double _x, double _y, double _hw, double _hh);

        bool inside(const AABB& aabb);
        bool inside(double _x, double _y);
        bool overlap(const AABB& aabb);
        AABB split(Quadrant _q);
        AABB split(double _x, double _y, Quadrant _q);
        Quadrant find_quadrant(double _x, double _y);

        bool operator==(const AABB& rhs) {
            return (x == rhs.x && y == rhs.y && hw == rhs.hw && hh == rhs.hh);
        }
        bool operator!=(const AABB& rhs) {
            return (x != rhs.x || y != rhs.y || hw != rhs.hw || hh != rhs.hh);
        }

        double x, y, hw, hh;
    };
}  // namespace scenegraph
#endif
