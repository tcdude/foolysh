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
 * :Animation implementation:
 *
 * :AnimationManager:
 *      Manages all active animations and resolves conflicts. Keeps a time-line
 *      of currently active animations, where each animation is viewed as a
 *      track. The AnimationManager naively uses a LIFO principle for conflict
 *      resolution, specifically, it deactivates conflicting animations and
 *      issues a warning to the user.
 *
 * :Interval:
 *      Node manipulation with user defined duration. Requires the end state,
 *      the start state can optionally be specified. If no start state is
 *      provided, the state of the node at first invocation of the interval will
 *      be used.
 *
 * :Animation:
 *      Node manipulation with user defined speed. Requires the end state,
 *      the start state can optionally be specified. If no start state is
 *      provided, the state of the node at first invocation of the interval will
 *      be used. Is transformed on first invocation into an Interval.
 *
 * :Sequence:
 *      Sequence of intervals and/or animations.
 *
 * Both ``Interval`` and ``Animation`` allow for easing in and out and can hold
 * a number of modifiers (pos, angle, ...) that are applied in parallel.
 *
 * Interval / Animation type_flags (+ = Interval Only):
 *  1       Start Pos (+)
 *  2       End Pos
 *  4       Start Angle (+)
 *  8       End Angle
 *  16      Start Rotation Center (+)
 *  32      End Rotation Center
 *  64      Start Scale (+)
 *  128     End Scale
 *  256     Start Depth (+)
 *  512     End Depth
 *  1024    Relative to Node
 */


#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include "list_t.hpp"
#include <utility>
#include <map>
#include <memory>
#include <vector>

namespace scenegraph {
    class Node;
    struct Scale;
}

namespace tools {
    class Vector2;
}

namespace animation {
    enum BlendType {
        NO_BLEND,
        EASE_IN,
        EASE_OUT,
        EASE_IN_OUT
    };

    inline double lerp(double pos, double total, BlendType blend);

    /**
     * Hold information for a positional animation.
     */
    struct PositionData {
        tools::Vector2 start, end;
        std::unique_ptr<scenegraph::Node> relative_node;
        bool active = false, has_start = false;
    };

    /**
     * Hold information for a scale animation.
     */
    struct ScaleData {
        scenegraph::Scale start, end;
        std::unique_ptr<scenegraph::Node> relative_node;
        bool active = false, has_start = false;
    };

    /**
     * Hold information for a rotational animation.
     */
    struct AngleData {
        double start, end;
        std::unique_ptr<scenegraph::Node> relative_node;
        bool active = false, has_start = false;
    };

    /**
     * Hold information for a depth animation.
     */
    struct DepthData {
        int start, end;
        std::unique_ptr<scenegraph::Node> relative_node;
        bool active = false, has_start = false;
    };

    class AnimationType {
    public:
        // Setup
        void set_node(scenegraph::Node n);
        void set_blend(BlendType b);

        void add_pos(tools::Vector2 end);
        void add_pos(tools::Vector2 end, scenegraph::Node relative_node);
        void add_pos(tools::Vector2 start, tools::Vector2 end);
        void add_pos(tools::Vector2 start, tools::Vector2 end,
                     scenegraph::Node relative_node);

        void add_scale(scenegraph::Scale end);
        void add_scale(scenegraph::Scale end, scenegraph::Node relative_node);
        void add_scale(scenegraph::Scale start, scenegraph::Scale end);
        void add_scale(scenegraph::Scale start, scenegraph::Scale end,
                       scenegraph::Node relative_node);

        void add_rotation(double end);
        void add_rotation(double end, scenegraph::Node relative_node);
        void add_rotation(double start, double end);
        void add_rotation(double start, double end,
                          scenegraph::Node relative_node);

        void add_rotation_center(tools::Vector2 end);
        void add_rotation_center(tools::Vector2 start, tools::Vector2 end);

        void add_depth(int end);
        void add_depth(int end, scenegraph::Node relative_node);
        void add_depth(int start, int end);
        void add_depth(int start, int end, scenegraph::Node relative_node);

        // Control
        virtual void reset();
        virtual void step(const double dt);
        double get_playback_pos();

    protected:
        double duration, playback_pos;
        scenegraph::Node node;
        BlendType blend;
        PositionData pos, center_pos;
        ScaleData scale;
        AngleData angle;
        DepthData depth;
    };

    class Interval : public AnimationType {
    public:
        Interval();
        void set_duration(const double d);

        void reset();
        void step(const double dt);
    };

    class Animation : public AnimationType {
    public:
        Animation();
        void set_pos_speed(const double s);
        void set_scale_speed(const double s);
        void set_rotation_speed(const double s);
        void set_rotation_center_speed(const double s);
        void set_depth_speed(const double s);

        void reset();
        void step(const double dt);
    };

    class Sequence {
    public:
    private:
    };

    class AnimationManager {
    public:
        void animate(const double dt);
    private:
        std::map<int, std::pair<int, int>> _active_animations;
        std::map<int, scenegraph::Node> _nodes;

        void _update_active(const double dt);
        void _execute_delta(const double dt);
    };

}  // namespace animation

#endif
