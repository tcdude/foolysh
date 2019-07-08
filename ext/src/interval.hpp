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
 * Interval
 */

#ifndef INTERVAL_HPP
#define INTERVAL_HPP

#include <vector>
#include <memory>

namespace tools {
    class Vector2;
}

namespace scenegraph {
    class Node;
}

namespace animation {
    enum IntervalType {
        POSITION = 1,
        SCALE = 2,
        SCALE_X = 4,
        SCALE_Y = 8,
        ROTATION = 16,
        ROTATION_CENTER = 32
    };

    enum BlendType {
        NONE,
        EASE_IN,
        EASE_OUT,
        EASE_IN_OUT
    };

    struct Position {
        tools::Vector2 start, end;
    };

    struct Scalar {
        double start, end;
    };

    class Interval {
    public:
        Interval(const scenegraph::Node& node, const double duration = 0.0,
                 BlendType blend_type = NONE);
        Interval(const Interval& other);
        void add_position(const tools::Vector2& start,
                          const tools::Vector2& end);
        void add_position(const tools::Vector2& end);
        void add_node_origin(const scenegraph::Node& other);
        void add_scale(const double start, const double end);
        void add_scale_x(const double start, const double end);
        void add_scale_y(const double start, const double end);
        void add_scale(const double sx, const double sy, const double ex,
                       const double ey);
        void add_rotation(const double start, const double end);
        void add_rotation_center(const tools::Vector2& start,
                                 const tools::Vector2& end);
        double execute_step(const double dt);
        void reset();
        tools::Vector2 get_position();
        double get_scale() const;
        double get_scale_x() const;
        double get_scale_y() const;
        double get_rotation() const;
        tools::Vector2 get_rotation_center() const;

    private:
        std::unique_ptr<scenegraph::Node> _node_ptr, _other_ptr;
        std::unique_ptr<Position> _pos_ptr, _rot_center_ptr;
        std::unique_ptr<Scalar> _scale_ptr, _sx_ptr, _sy_ptr, _rot_ptr;
        char _type_flags;
        const BlendType _blend_type;
        const double _duration;
        double _delta, _t;
    };

}   // namespace animation


#endif
