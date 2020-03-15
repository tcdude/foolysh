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
#include "vec2.hpp"
#include "node.hpp"
#include <utility>
#include <map>
#include <memory>
#include <vector>


namespace foolysh {
namespace animation {
    typedef foolysh::tools::Vec2 Vec2;
    typedef foolysh::scene::Node Node;
    typedef foolysh::scene::Scale Scale;
    using foolysh::tools::ExtFreeList;

    enum BlendType {
        NO_BLEND,
        EASE_IN,
        EASE_OUT,
        EASE_IN_OUT
    };

    /**
    * Linear interpolation of two values to the range [0, 1] with optional
    * blending.
    */
    inline double lerp(double pos, double total, BlendType blend) {
        if (pos >= total) {
            return 1.0;
        }
        if (pos == 0.0) {
            return 0.0;
        }

        double v = pos / total;

        switch (blend) {
            case NO_BLEND: {
                return v;
            }
            case EASE_IN: {
                double vsq = v * v;
                return ((3.0 * vsq) - (vsq * v)) * 0.5;
            }
            case EASE_OUT: {
                double vsq = v * v;
                return ((3.0 * v) - (vsq * v)) * 0.5;
            }
            case EASE_IN_OUT: {
                double vsq = v * v;
                return (3.0 * vsq) - (2.0 * v * vsq);
            }
        }

        // throw std::runtime_error("Unknown blend_type");
        std::abort();
    }

    /**
     * Hold information for a positional animation.
     */
    struct PositionData {
        Vec2 start, end;
        std::unique_ptr<Node> relative_node;
        bool active = false, has_start = false;
    };

    /**
     * Hold information for a scale animation.
     */
    struct ScaleData {
        Scale start, end;
        std::unique_ptr<Node> relative_node;
        bool active = false, has_start = false;
    };

    /**
     * Hold information for a rotational animation.
     */
    struct AngleData {
        double start, end;
        std::unique_ptr<Node> relative_node;
        bool active = false, has_start = false;
    };

    /**
     * Hold information for a depth animation.
     */
    struct DepthData {
        int start, end;
        std::unique_ptr<Node> relative_node;
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
        std::unique_ptr<Node> node;
        BlendType blend = NO_BLEND;
        PositionData pos, center_pos;   // flags: 1, 2
        ScaleData scale;                // flags: 4
        AngleData angle;                // flags: 8
        DepthData depth;                // flags: 16
        int _ref_count, animation_id;

        static ExtFreeList<AnimationData*> _ad;
    };
    ExtFreeList<AnimationData*> AnimationData::_ad;

    typedef std::map<int, char> ActiveAnimationMap;

    /**
     * Base class for Animation, Interval and Sequence (playable objects).
     */
    class AnimationBase {
    public:
        virtual void reset();
        virtual double step(const double dt, ActiveAnimationMap& aam);
        virtual std::unique_ptr<AnimationBase> get_copy();
        virtual void loop(const bool l);
    };

    /**
     * Base class for animations, providing setup and control methods and holds
     * shared data.
     */
    class AnimationType : public AnimationBase {
    public:
        AnimationType();
        ~AnimationType();
        AnimationType(const AnimationType& other);
        AnimationType(AnimationType&& other) noexcept;
        AnimationType& operator=(const AnimationType& other);
        AnimationType& operator=(AnimationType&& other) noexcept;

        // Setup
        void set_node(Node& n);
        void set_blend(BlendType b);

        void add_pos(Vec2 end);
        void add_pos(Vec2 end, Node relative_node);
        void add_pos(Vec2 start, Vec2 end);
        void add_pos(Vec2 start, Vec2 end,
                     Node relative_node);

        void add_scale(Scale end);
        void add_scale(Scale end, Node relative_node);
        void add_scale(Scale start, Scale end);
        void add_scale(Scale start, Scale end,
                       Node relative_node);

        void add_rotation(double end);
        void add_rotation(double end, Node relative_node);
        void add_rotation(double start, double end);
        void add_rotation(double start, double end,
                          Node relative_node);

        void add_rotation_center(Vec2 end);
        void add_rotation_center(Vec2 start, Vec2 end);

        void add_depth(int end);
        void add_depth(int end, Node relative_node);
        void add_depth(int start, int end);
        void add_depth(int start, int end, Node relative_node);

        // Control
        void reset();
        void loop(const bool l) {}
        double get_playback_pos();
        int node_id();

    protected:
        AnimationData& _get_animation_data(const int animation_id);
        virtual char active_animations();
        int _animation_id;
    };

    /**
     * Interval with fixed duration.
     */
    class Interval : public AnimationType {
    public:
        void set_duration(const double d);

        void reset();
        double step(const double dt, ActiveAnimationMap& aam);
        std::unique_ptr<AnimationBase> get_copy();

    protected:
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
        double step(const double dt, ActiveAnimationMap& aam);
        std::unique_ptr<AnimationBase> get_copy();

    protected:
        char active_animations();
    };

    /**
     * Container to hold a sequence of ``AnimationType`` objects.
     */
    class Sequence : public AnimationBase {
    public:
        void append(std::unique_ptr<AnimationBase>& a);
        void reset();
        double step(const double dt, ActiveAnimationMap& aam);
        void loop(const bool l);
        std::unique_ptr<AnimationBase> get_copy();
    private:
        std::vector<std::unique_ptr<AnimationBase>> _v;
        std::vector<std::unique_ptr<AnimationBase>>::size_type _active = 0;
        bool _loop = false;
    };

    /**
     * Custom Types:
     * ActiveAnimationMap:
     *      std::map<int, char> holding flags of currently active animations.
     *      1 = position
     *      2 = rotation center
     *      4 = scale
     *      8 = angle
     *      16 = depth
     * AnimationStatusMap:
     *      std::map<int, char> holding playback status for animations.
     *      0 = stopped / uninitialized
     *      1 = paused
     *      2 = start playback
     *      3 = playing
     *      4 = conflict
     */
    typedef ActiveAnimationMap AnimationStatusMap;
    typedef std::map<int, std::unique_ptr<AnimationBase>, std::greater<int>> AnimationMap;

    /**
     * Interface to control animation. Provides methods to create Interval,
     * Animation and Sequence instances, managed by the AnimationManager.
     */
    class AnimationManager {
    public:
        int new_interval();
        int new_animation();
        int new_sequence();
        Interval& get_interval(const int i_id);
        Animation& get_animation(const int a_id);
        Sequence& get_sequence(const int s_id);
        std::unique_ptr<AnimationBase>& get_animation_base_ptr(const int i_id);
        void remove_interval(const int i_id);
        void remove_animation(const int a_id);
        void remove_sequence(const int s_id);
        void play_interval(const int i_id);
        void play_animation(const int a_id);
        void play_sequence(const int s_id);
        void pause_interval(const int i_id);
        void pause_animation(const int a_id);
        void pause_sequence(const int s_id);
        void resume_interval(const int i_id);
        void resume_animation(const int a_id);
        void resume_sequence(const int s_id);
        void stop_interval(const int i_id);
        void stop_animation(const int a_id);
        void stop_sequence(const int s_id);
        char get_interval_status(const int i_id);
        char get_animation_status(const int a_id);
        char get_sequence_status(const int s_id);
        void append(const int s_id, const int a_id);

        void animate(const double dt);

    private:
        ActiveAnimationMap _aam;
        AnimationMap _anims;
        AnimationStatusMap _anim_status;
        int _max_anim = -1;
    };

}  // namespace animation
}  // namespace foolysh

#endif
