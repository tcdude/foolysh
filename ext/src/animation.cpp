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
 */

#include <stdexcept>
#include <algorithm>

#include "animation.hpp"
#include "node.hpp"
#include "vector2.hpp"


/**
 * Linear interpolation of two values to the range [0, 1] with optional
 * blending.
 */
inline double animation::
lerp(double pos, double total, BlendType blend) {
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

    throw std::runtime_error("Unknown blend_type");
}


// AnimationData

/**
 *
 */
animation::AnimationData::
AnimationData(){
    _ref_count = 1;
}


// AnimationType Rule of 5

/**
 * Default Constructor: Create a new AnimationData instance.
 */
animation::AnimationType::
AnimationType() {
    _animation_id = AnimationData::_ad.insert(new AnimationData());
    AnimationData::_ad[_animation_id]->animation_id = _animation_id;

}

/**
 * Destructor: Decrements AnimationData ref_count, deletes AnimationData if
 * ref_count drops to zero.
 */
animation::AnimationType::
~AnimationType() {
    if (_animation_id > -1) {
        AnimationData& ad = _get_animation_data(_animation_id);
        if (ad._ref_count == 1) {
            /* If ours is the last reference, delete the AnimationData */
            AnimationData::_ad.erase(_animation_id);
        }
        else {
            --ad._ref_count;
        }
    }
}

/**
 * Copy Constructor. Creates a copy of AnimationType ``other`` with access to
 * the same AnimationData instance.
 */
animation::AnimationType::
AnimationType(const AnimationType& other) {
    _animation_id = other._animation_id;
    AnimationData& ad = _get_animation_data(_animation_id);
    ++ad._ref_count;
}

/**
 * Move Constructor. Invalidates ``other``.
 */
animation::AnimationType::
AnimationType(AnimationType&& other) noexcept {
    _animation_id = other._animation_id;
    other._animation_id = -1;
}

/**
 * Copy Assignment Operator. Creates a copy of AnimationType ``other`` with
 * access to the same AnimationData instance.
 */
animation::AnimationType& animation::AnimationType::
operator=(const AnimationType& other) {
    if (AnimationData::_ad.active(_animation_id)) {
        AnimationData& ad = _get_animation_data(_animation_id);
        --ad._ref_count;
    }
    _animation_id = other._animation_id;
    AnimationData& ad = _get_animation_data(_animation_id);
    ++ad._ref_count;
    return *this;
}

/**
 * Move Assignment Operator. Invalidates ``other``.
 */
animation::AnimationType& animation::AnimationType::
operator=(AnimationType&& other) noexcept {
    if (AnimationData::_ad.active(_animation_id)) {
        AnimationData& ad = _get_animation_data(_animation_id);
        --ad._ref_count;
    }
    _animation_id = other._animation_id;
    other._animation_id = -1;
    return *this;
}


// AnimationType - Setup

/**
 * Set the node, the animation will apply to.
 */
void animation::AnimationType::
set_node(scenegraph::Node n) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.node = n;
}

/**
 * Set the BlendType, used for the animation.
 */
void animation::AnimationType::
set_blend(BlendType b) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.blend = b;
}

/**
 * Add a position animation, only specifying the end point.
 */
void animation::AnimationType::
add_pos(tools::Vector2 end) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.pos.active = true;
    ad.pos.end = end;
    ad.pos.has_start = false;
    if (ad.pos.relative_node) {
        ad.pos.relative_node.reset(nullptr);
    }
}

/**
 * Add a position animation relative to a Node, specifying the end point and a
 * Node.
 */
void animation::AnimationType::
add_pos(tools::Vector2 end, scenegraph::Node relative_node) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.pos.active = true;
    ad.pos.end = end;
    ad.pos.has_start = false;
    ad.pos.relative_node.reset(scenegraph::Node(relative_node));
}

/**
 * Add a position animation, specifying start and end points.
 */
void animation::AnimationType::
add_pos(tools::Vector2 start, tools::Vector2 end) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.pos.active = true;
    ad.pos.start = start;
    ad.pos.end = end;
    ad.pos.has_start = true;
    if (ad.pos.relative_node) {
        ad.pos.relative_node.reset(nullptr);
    }
}

/**
 * Add a position animation relative to a Node, specifying the start, end point
 * and a Node.
 */
void animation::AnimationType::
add_pos(tools::Vector2 start, tools::Vector2 end,
        scenegraph::Node relative_node) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.pos.active = true;
    ad.pos.start = start;
    ad.pos.end = end;
    ad.pos.has_start = true;
    ad.pos.relative_node.reset(scenegraph::Node(relative_node));
}

/**
 * Add a scale animation, only specifying the end scale.
 */
void animation::AnimationType::
add_scale(scenegraph::Scale end) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.scale.active = true;
    ad.scale.end = end;
    ad.scale.has_start = false;
    if (ad.scale.relative_node) {
        ad.scale.relative_node.reset(nullptr);
    }
}

/**
 * Add a scale animation relative to a Node, specifying the end scale and a
 * Node.
 */
void animation::AnimationType::
add_scale(scenegraph::Scale end, scenegraph::Node relative_node) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.scale.active = true;
    ad.scale.end = end;
    ad.scale.has_start = false;
    ad.scale.relative_node.reset(new scenegraph::Node(relative_node));
}

/**
 * Add a scale animation, specifying the start and end scale.
 */
void animation::AnimationType::
add_scale(scenegraph::Scale start, scenegraph::Scale end) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.scale.active = true;
    ad.scale.start = start;
    ad.scale.end = end;
    ad.scale.has_start = true;
    if (ad.scale.relative_node) {
        ad.scale.relative_node.reset(nullptr);
    }
}

/**
 * Add a scale animation relative to a Node, specifying the start and end scale
 * and a Node.
 */
void animation::AnimationType::
add_scale(scenegraph::Scale start, scenegraph::Scale end,
          scenegraph::Node relative_node) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.scale.active = true;
    ad.scale.start = start;
    ad.scale.end = end;
    ad.scale.has_start = false;
    ad.scale.relative_node.reset(new scenegraph::Node(relative_node));
}

/**
 * Add a rotation animation, only specifying the end angle in degrees.
 */
void animation::AnimationType::
add_rotation(double end) {
    angle.active = true;
    angle.end = end;
    angle.has_start = false;
    if (angle.relative_node) {
        angle.relative_node.reset(nullptr);
    }
}

/**
 * Add a rotation animation relative to a Node, specifying the end angle in
 * degrees and a Node.
 */
void animation::AnimationType::
add_rotation(double end, scenegraph::Node relative_node) {
    angle.active = true;
    angle.end = end;
    angle.has_start = false;
    angle.relative_node.reset(new scenegraph::Node(relative_node));
}

/**
 * Add a rotation animation, specifying the start and end angle in degrees.
 */
void animation::AnimationType::
add_rotation(double start, double end) {
    angle.active = true;
    angle.start = start;
    angle.end = end;
    angle.has_start = true;
    if (angle.relative_node) {
        angle.relative_node.reset(nullptr);
    }
}

/**
 * Add a rotation animation relative to a Node, specifying the start and end
 * angle in degrees and a Node.
 */
void animation::AnimationType::
add_rotation(double start, double end, scenegraph::Node relative_node) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.angle.active = true;
    ad.angle.end = start;
    ad.angle.end = end;
    ad.angle.has_start = true;
    ad.angle.relative_node.reset(new scenegraph::Node(relative_node));
}

/**
 * Add a position animation for the rotation center, only specifying the end
 * point.
 */
void animation::AnimationType::
add_rotation_center(tools::Vector2 end) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.center_pos.active = true;
    ad.center_pos.end = end;
    ad.center_pos.has_start = false;
}

/**
 * Add a position animation for the rotation center, specifying the end and
 * start point.
 */
void animation::AnimationType::
add_rotation_center(tools::Vector2 start, tools::Vector2 end) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.center_pos.active = true;
    ad.center_pos.start = start;
    ad.center_pos.end = end;
    ad.center_pos.has_start = true;
}

/**
 * Add a depth animation, only specifying the end depth.
 */
void animation::AnimationType::
add_depth(int end) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.depth.active = true;
    ad.depth.end = end;
    ad.depth.has_start = false;
    if (ad.depth.relative_node) {
        ad.depth.relative_node.reset(nullptr);
    }
}

/**
 * Add a depth animation relative to a Node, specifying the end depth and a
 * Node.
 */
void animation::AnimationType::
add_depth(int end, scenegraph::Node relative_node) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.depth.active = true;
    ad.depth.end = end;
    ad.depth.has_start = false;
    ad.depth.relative_node.reset(new scenegraph::Node(relative_node));
}

/**
 * Add a depth animation, specifying the start and end depth.
 */
void animation::AnimationType::
add_depth(int start, int end) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.depth.active = true;
    ad.depth.start = start;
    ad.depth.end = end;
    ad.depth.has_start = true;
    if (ad.depth.relative_node) {
        ad.depth.relative_node.reset(nullptr);
    }
}

/**
 * Add a depth animation relative to a Node, specifying the start and end depth
 * and a Node.
 */
void animation::AnimationType::
add_depth(int start, int end, scenegraph::Node relative_node) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.depth.active = true;
    ad.depth.start = start;
    ad.depth.end = end;
    ad.depth.has_start = true;
    ad.depth.relative_node.reset(new scenegraph::Node(relative_node));
}


// AnimationType - Control

/**
 * Reset command that needs to be overridden in subclasses.
 */
virtual void animation::AnimationType::
reset() {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.playback_pos = -1.0;
    if (ad.pos.active && !ad.pos.has_start) {
        if (ad.pos.relative_node) {
            ad.pos.start = ad.node.get_pos(ad.pos.relative_node);
        }
        else {
            ad.pos.start = ad.node.get_pos();
        }
    }

    if (ad.center_pos.active && !ad.center_pos.has_start) {
        ad.pos.start = ad.node.get_rotation_center();
    }

    if (ad.scale.active && !ad.scale.has_start) {
        if (ad.scale.relative_node) {
            ad.scale.start = ad.node.get_scale(ad.scale.relative_node);
        }
        else {
            ad.scale.start = ad.node.get_scale();
        }
    }

    if (ad.angle.active && !ad.angle.has_start) {
        if (ad.angle.relative_node) {
            ad.angle.start = ad.node.get_angle(ad.angle.relative_node);
        }
        else {
            ad.angle.start = ad.node.get_angle();
        }
    }

    if (ad.depth.active && !ad.depth.has_start) {
        if (ad.depth.relative_node) {
            ad.depth.start = ad.node.get_depth(ad.depth.relative_node);
        }
        else {
            ad.depth.start = ad.node.get_depth();
        }
    }
}

/**
 * Step command that needs to be overridden in subclasses.
 */
virtual double animation::AnimationType::
step(const double dt) {
    return 0.0;
}

/**
 * Returns the current playback position in seconds.
 */
double animation::AnimationType::
get_playback_pos() {
    AnimationData& ad = _get_animation_data(_animation_id);
    return ad.playback_pos;
}

/**
 *
 */
virtual std::unique_ptr<animation::AnimationType> animation::AnimationType::
get_copy() {
    std::unique_ptr<AnimationType> ptr;
    ptr.reset(new AnimationType());
    AnimationData& ad = _get_animation_data();
    AnimationData& ptr_ad = ptr->_get_animation_data();

    ptr_ad.duration = ad.duration;
    ptr_ad.playback_pos = ad.playback_pos;
    ptr_ad.pos_speed = ad.pos_speed;
    ptr_ad.scale_speed = ad.scale_speed;
    ptr_ad.rotation_speed = ad.rotation_speed;
    ptr_ad.rotation_center_speed = ad.rotation_center_speed;
    ptr_ad.depth_speed = ad.depth_speed;
    ptr_ad.dur_pos = ad.dur_pos;
    ptr_ad.dur_scalex = ad.dur_scalex;
    ptr_ad.dur_scaley = ad.dur_scaley;
    ptr_ad.dur_angle = ad.dur_angle;
    ptr_ad.dur_center_pos = ad.dur_center_pos;
    ptr_ad.dur_depth = ad.dur_depth;
    ptr_ad.node = ad.node;
    ptr_ad.blend = ad.blend;
    ptr_ad.pos = ad.pos;
    ptr_ad.center_pos = ad.center_pos;
    ptr_ad.scale = ad.scale;
    ptr_ad.angle = ad.angle;
    ptr_ad.depth = ad.depth;
    return ptr;
}

/**
 * Helper method to retrieve the AnimationData for the specified
 * ``animation_id``.
 */
animation::AnimationData& animation::AnimationType::
_get_animation_data(const int animation_id) {
    if (!AnimationData::_ad.active(animation_id)) {
        throw std::logic_error("Tried to access invalid AnimationData");
    }
    return *AnimationData::_ad[animation_id];
}


// Interval

/**
 * Set the duration in seconds.
 */
void animation::Interval::
set_duration(const double d) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.duration = d;
}

/**
 * Reset to initial state. Updates start states, for the active animation types,
 * where none have been explicitly specified.
 */
void animation::Interval::
reset() {
    AnimationType::reset();
}

/**
 * Returns -1.0 if the Interval is not finished, otherwise returns the amount
 * of ``dt`` seconds remaining after the Interval was complete.
 */
double animation::Interval::
step(const double dt) {
    AnimationData& ad = _get_animation_data(_animation_id);

    if (ad.playback_pos == -1.0) {
        ad.playback_pos = 0.0;
    }

    ad.playback_pos += dt;
    if (ad.playback_pos >= ad.duration) {
        _update(1.0);
        return ad.playback_pos - ad.duration;
    }

    _update(lerp(ad.playback_pos, ad.duration, ad.blend));

    return -1.0;
}

/**
 * Update the underlying Node
 */
void animation::Interval::
_update(const double prog) {
    // position
    if (ad.pos.active) {
        if (ad.pos.relative_node) {
            ad.node.set_pos(
                ad.pos.relative_node,
                (ad.pos.end - ad.pos.start) * prog + ad.pos.start);
        }
        else {
            ad.node.set_pos((ad.pos.end - ad.pos.start) * prog + ad.pos.start);
        }
    }

    // rotation_center
    if (ad.center_pos.active) {
        ad.node.set_rotation_center(
            (ad.center_pos.end - ad.center_pos.start)
            * prog + ad.center_pos.start);
    }

    // scale
    if (ad.scale.active) {
        if (ad.scale.relative_node) {
            ad.node.set_scale(
                ad.scale.relative_node,
                (ad.scale.end - ad.scale.start) * prog + ad.scale.start);
        }
        else {
            ad.node.set_scale(
                (ad.scale.end - ad.scale.start) * prog + ad.scale.start);
        }
    }

    // rotation
    if (ad.angle.active) {
        if (ad.angle.relative_node) {
            ad.node.set_angle(
                ad.angle.relative_node,
                (ad.angle.end - ad.angle.start) * prog + ad.angle.start);
        }
        else {
            ad.node.set_angle(
                (ad.angle.end - ad.angle.start) * prog + ad.angle.start);
        }
    }

    // depth
    if (ad.depth.active) {
        if (ad.depth.relative_node) {
            ad.node.set_depth(
                ad.depth.relative_node,
                (ad.depth.end - ad.dept.start) * prog + ad.depth.start);
        }
        else {
            ad.node.set_depth(
                (ad.depth.end - ad.dept.start) * prog + ad.depth.start);
        }
    }
}

/**
 *
 */
std::unique_ptr<animation::AnimationType> animation::Interval::
get_copy() {
    return AnimationType::get_copy();
}


// Animation

/**
 * Reset to initial state. Updates start states, for the active animation types,
 * where none have been explicitly specified and translates speed into duration.
 */
void animation::Animation::
reset() {
    AnimationType::reset();
    AnimationData& ad = _get_animation_data(_animation_id);
    double duration = -1.0;

    if (ad.pos.active) {
        if (ad.pos_speed < 0.0) {
            throw std::logic_error("Position animation specified without
                                    speed");
        }
        double tmp_d = (ad.pos.end - ad.pos.start).length() / ad.pos_speed;
        ad.dur_pos = tmp_d;
        if (tmp_d > duration) {
            duration = tmp_d;
        }
    }

    if (ad.center_pos.active) {
        if (ad.rotation_center_speed < 0.0) {
            throw std::logic_error("Rotation center animation specified without
                                    speed");
        }
        double tmp_d = (ad.center_pos.end - ad.center_pos.start).length()
                       / ad.rotation_center_speed;
        ad.dur_center_pos = tmp_d;
        if (tmp_d > duration) {
            duration = tmp_d;
        }
    }

    if (ad.scale.active) {
        if (ad.scale_speed < 0.0) {
            throw std::logic_error("Scale animation specified without
                                    speed");
        }
        ad.dur_scalex = std::abs(
            ad.scale.end.sx - ad.scale.start.sx) / ad.scale_speed;

        ad.dur_scaley = std::abs(
            ad.scale.end.sy - ad.scale.start.sy) / ad.scale_speed;
        double tmp_d = std::max(ad.dur_scalex, ad.dur_scaley);
        if (tmp_d > duration) {
            duration = tmp_d;
        }
    }

    if (ad.angle.active) {
        if (ad.rotation_speed == 0.0) {
            throw std::logic_error("Rotation animation specified without
                                    speed");
        }
        ad.dur_angle = std::abs(
            ad.angle.end - ad.angle.start) / ad.rotation_speed;
        if (ad.dur_angle > duration) {
            duration = tmp_d;
        }
    }

    if (ad.depth.active) {
        if (ad.depth_speed < 0.0) {
            throw std::logic_error("Depth animation specified without
                                    speed");
        }
        ad.dur_depth = std::abs(
            ad.depth.end - ad.depth.start) / ad.depth_speed;
        if (tmp_d > duration) {
            duration = tmp_d;
        }
    }
}

/**
 * Returns -1.0 if the Animation is not finished, otherwise returns the amount
 * of ``dt`` seconds remaining after the Animation was complete.
 */
double animation::Animation::
step(const double dt) {
    AnimationData& ad = _get_animation_data(_animation_id);
    if (ad.playback_pos == -1.0) {
        ad.playback_pos = 0.0;
        return -1.0;
    }

    double rdt = -1.0;
    ad.playback_pos += dt;

    // position
    if (ad.pos.active) {
        tools::Vector2 p = (ad.pos.relative_node)
                        ? ad.node.get_pos(ad.pos.relative_node)
                        : ad.node.get_pos();
        if (p != ad.pos.end) {
            if (ad.playback_pos >= ad.dur_pos) {
                if (ad.pos.relative_node) {
                    ad.node.set_pos(ad.pos.relative_node, ad.pos.end);
                }
                else {
                    ad.node.set_pos(ad.pos.end);
                }
            }
            else {
                double prog = lerp(ad.playback_pos, ad.dur_pos, ad.blend);
                if (ad.pos.relative_node) {
                    ad.node.set_pos(
                        ad.pos.relative_node,
                        (ad.pos.end - ad.pos.start) * prog + ad.pos.start);
                }
                else {
                    ad.node.set_pos(
                        (ad.pos.end - ad.pos.start) * prog + ad.pos.start);
                }
            }
        }
    }

    // rotation_center
    if (ad.center_pos.active) {
        tools::Vector2 p = ad.node.get_rotation_center();
        if (p != ad.center_pos.end) {
            if (ad.playback_pos >= ad.dur_center_pos) {
                ad.node.set_rotation_center(ad.center_pos.end);
            }
            else {
                double prog = lerp(ad.playback_pos, ad.dur_center_pos,
                    ad.blend);
                ad.node.set_rotation_center(
                    (ad.center_pos.end - ad.center_pos.start) * prog
                    + ad.center_pos.start);
            }
        }
    }

    // scale
    if (ad.scale.active) {
        scenegraph::Scale s = (ad.scale.relative_node)
                            ? ad.node.get_scale(ad.scale.relative_node)
                            : ad.node.get_scale();
        scenegraph::Scale new_scale = s;
        if (s.sx != ad.scale.end.sx) {
            if (ad.playback_pos >= ad.dur_scalex) {
                new_scale.sx = ad.scale.end.sx;
            }
            else {
                double prog = lerp(ad.playback_pos, ad.dur_scalex, ad.blend);
                new_scale.sx = (ad.scale.end.sx - ad.scale.end.sx) * prog
                    + ad.scale.start.sx;
            }
        }
        if (s.sy != ad.scale.end.sy) {
            if (ad.playback_pos >= ad.dur_scaley) {
                new_scale.sy = ad.scale.end.sy;
            }
            else {
                double prog = lerp(ad.playback_pos, ad.dur_scaley, ad.blend);
                new_scale.sx = (ad.scale.end.sy - ad.scale.end.sy) * prog
                    + ad.scale.start.sy;
            }
        }

        if (s != new_scale) {
            if (ad.scale.relative_node) {
                ad.node.set_scale(ad.scale.relative_node, new_scale);
            }
            else {
                ad.node.set_scale(new_scale);
            }
        }
        else {
            rdt = -1.0;
        }
    }

    // rotation
    if (ad.angle.active) {
        double a = (ad.angle.relative_node)
                    ? ad.node.get_angle(ad.angle.relative_node)
                    : ad.node.get_angle();
        if (a != ad.angle.end) {
            if (ad.playback_pos >= ad.dur_angle) {
                if (ad.angle.relative_node) {
                    ad.node.set_angle(ad.angle.relative_node, ad.angle.end);
                }
                else {
                    ad.node.set_angle(ad.angle.end);
                }
            }
            else {
                double prog = lerp(ad.playback_pos, ad.dur_angle, ad.blend);
                if (ad.angle.relative_node) {
                    ad.node.set_angle(
                        ad.angle.relative_node,
                        (ad.angle.end - ad.angle.start) * prog + ad.angle.start
                    );
                }
                else {
                    ad.node.set_angle(
                        (ad.angle.end - ad.angle.start) * prog + ad.angle.start
                    );
                }
            }
        }
    }

    // depth
    if (ad.depth.active) {
        int depth = (ad.depth.relative_node)
                    ? ad.node.get_depth(ad.depth.relative_node)
                    : ad.node.get_depth();
        if (depth != ad.depth.end) {
            if (ad.playback_pos >= ad.dur_depth) {
                if (ad.depth.relative_node) {
                    ad.node.set_depth(ad.depth.relative_node, ad.depth.end);
                }
                else {
                    ad.node.set_depth(ad.depth.end);
                }
            }
            else {
                double prog = lerp(ad.playback_pos, ad.dur_depth, ad.blend);
                if (ad.depth.relative_node) {
                    ad.node.set_depth(
                        ad.depth.relative_node,
                        (ad.depth.end - ad.depth.start) * prog + ad.depth.start
                    );
                }
                else {
                    ad.node.set_depth(
                        (ad.depth.end - ad.depth.start) * prog + ad.depth.start
                    );
                }
            }
        }
    }
    if (ad.playback_pos >= ad.duration) {
        return ad.playback_pos - ad.duration;
    }
    return -1.0;
}

/**
 *
 */
std::unique_ptr<animation::AnimationType> animation::Animation::
get_copy() {
    return AnimationType::get_copy();
}


// Sequence

/**
 *
 */
void animation::Sequence::
append(animation::AnimationType& a) {
    _v.push_back(a.get_copy());
}

/**
 *
 */
void animation::Sequence::
reset() {
    _active = -1;
}

/**
 *
 */
double animation::Sequence::
step(const double dt) {
    if (_active < 0) {
        _active = 0;
        _v[0]->reset();
    }
    double rdt = _v[_active]->step(dt);
    if (rdt >= 0.0) {
        ++_active;
        if (_active == _v.size()) {
            return rdt;
        }
        return step(rdt);
    }
    return rdt;
}
