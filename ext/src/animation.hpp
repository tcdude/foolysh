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

    /**
     * Reference counted Type, holding data for AnimationType.
     */
    struct AnimationData {
        AnimationData();
        ~AnimationData();

        double duration, playback_pos = -1.0;
        double pos_speed = -1.0, scale_speed = -1.0, rotation_speed = 0.0,
            rotation_center_speed = -1.0, depth_speed = -1.0;
        double dur_pos, dur_scalex, dur_scaley, dur_angle, dur_center_pos,
            dur_depth;
        scenegraph::Node node;
        BlendType blend;
        PositionData pos, center_pos;   // flags: 1, 2
        ScaleData scale;                // flags: 4
        AngleData angle;                // flags: 8
        DepthData depth;                // flags: 16
        int _ref_count, animation_id;

        static ExtFreeList<AnimationData*> _ad;
    };
    ExtFreeList<AnimationData*> AnimationData::_ad;


    /**
     * Base class for animations, providing setup and control methods and holds
     * shared data.
     */
    class AnimationType {
    public:
        AnimationType();
        ~AnimationType();
        AnimationType(const AnimationType& other);
        AnimationType(AnimationType&& other) noexcept;
        AnimationType& operator=(const AnimationType& other);
        AnimationType& operator=(AnimationType&& other) noexcept;

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
        virtual double step(const double dt);
        double get_playback_pos();
        virtual std::unique_ptr<AnimationType> get_copy();
        virtual char active_animations();

    protected:
        AnimationData& _get_animation_data(const int animation_id);
        int _animation_id;
    };

    /**
     * Interval with fixed duration.
     */
    class Interval : public AnimationType {
    public:
        void set_duration(const double d);

        void reset();
        double step(const double dt);
        std::unique_ptr<AnimationType> get_copy();
        char active_animations();

    private:
        void _update(const double prog);
    };

    /**
     * Animation with fixed speed, runs until all end states are reached.
     */
    class Animation : public AnimationType {
    public:
        void set_pos_speed(const double s);
        void set_scale_speed(const double s);
        void set_rotation_speed(const double s);
        void set_rotation_center_speed(const double s);
        void set_depth_speed(const double s);

        void reset();
        double step(const double dt);
        std::unique_ptr<AnimationType> get_copy();
        char active_animations();
    };

    /**
     * Container to hold a sequence of ``AnimationType`` objects.
     */
    class Sequence {
    public:
        void append(AnimationType& a);
        void reset();
        double step(const double dt);
        char active_animations();
    private:
        std::vector<std::unique_ptr<AnimationType>> _v;
        int _active = -1;
        char _active_anim = 0;
    };

    /**
     * Interface to control animation.
     */
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
