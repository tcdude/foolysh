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

#include <stdexcept>
#include <algorithm>

#include "animation.hpp"

namespace foolysh {
namespace animation {

// AnimationData

/**
 *
 */
AnimationData::
AnimationData(){
    _ref_count = 1;
}

// AnimationBase

/**
 * Reset command that needs to be overridden in subclasses.
 */
void AnimationBase::
reset() {
    throw std::runtime_error("Cannot be called from AnimationBase class.");
}

/**
 * Step command that needs to be overridden in subclasses.
 */
double AnimationBase::
step(const double dt, ActiveAnimationMap& aam) {
    return 0.0;
}

/**
 * Retrieve a copy of the Animation. Needs to be overridden in subclasses.
 */
std::unique_ptr<AnimationBase> AnimationBase::
get_copy() {
    throw std::runtime_error("Cannot be called from AnimationBase class.");
}

/**
 * Virtual method to set looping behavior. Currently only implemented for
 * Sequence.
 */
void AnimationBase::
loop(const bool l) {
    throw std::runtime_error("Invalid sub-type for loop().");
}


// AnimationType Rule of 5

/**
 * Default Constructor: Create a new AnimationData instance.
 */
AnimationType::
AnimationType() {
    _animation_id = AnimationData::_ad.insert(new AnimationData());
    AnimationData::_ad[_animation_id]->animation_id = _animation_id;

}

/**
 * Destructor: Decrements AnimationData ref_count, deletes AnimationData if
 * ref_count drops to zero.
 */
AnimationType::
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
AnimationType::
AnimationType(const AnimationType& other) {
    _animation_id = other._animation_id;
    AnimationData& ad = _get_animation_data(_animation_id);
    ++ad._ref_count;
}

/**
 * Move Constructor. Invalidates ``other``.
 */
AnimationType::
AnimationType(AnimationType&& other) noexcept {
    _animation_id = other._animation_id;
    other._animation_id = -1;
}

/**
 * Copy Assignment Operator. Creates a copy of AnimationType ``other`` with
 * access to the same AnimationData instance.
 */
AnimationType& AnimationType::
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
AnimationType& AnimationType::
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
void AnimationType::
set_node(Node& n) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.node.reset(new Node(n));
}

/**
 * Set the BlendType, used for the animation.
 */
void AnimationType::
set_blend(BlendType b) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.blend = b;
}

/**
 * Add a position animation, only specifying the end point.
 */
void AnimationType::
add_pos(Vec2 end) {
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
void AnimationType::
add_pos(Vec2 end, Node relative_node) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.pos.active = true;
    ad.pos.end = end;
    ad.pos.has_start = false;
    ad.pos.relative_node.reset(new Node(relative_node));
}

/**
 * Add a position animation, specifying start and end points.
 */
void AnimationType::
add_pos(Vec2 start, Vec2 end) {
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
void AnimationType::
add_pos(Vec2 start, Vec2 end,
        Node relative_node) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.pos.active = true;
    ad.pos.start = start;
    ad.pos.end = end;
    ad.pos.has_start = true;
    ad.pos.relative_node.reset(new Node(relative_node));
}

/**
 * Add a scale animation, only specifying the end scale.
 */
void AnimationType::
add_scale(Scale end) {
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
void AnimationType::
add_scale(Scale end, Node relative_node) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.scale.active = true;
    ad.scale.end = end;
    ad.scale.has_start = false;
    ad.scale.relative_node.reset(new Node(relative_node));
}

/**
 * Add a scale animation, specifying the start and end scale.
 */
void AnimationType::
add_scale(Scale start, Scale end) {
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
void AnimationType::
add_scale(Scale start, Scale end,
          Node relative_node) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.scale.active = true;
    ad.scale.start = start;
    ad.scale.end = end;
    ad.scale.has_start = false;
    ad.scale.relative_node.reset(new Node(relative_node));
}

/**
 * Add a rotation animation, only specifying the end angle in degrees.
 */
void AnimationType::
add_rotation(double end) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.angle.active = true;
    ad.angle.end = end;
    ad.angle.has_start = false;
    if (ad.angle.relative_node) {
        ad.angle.relative_node.reset(nullptr);
    }
}

/**
 * Add a rotation animation relative to a Node, specifying the end angle in
 * degrees and a Node.
 */
void AnimationType::
add_rotation(double end, Node relative_node) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.angle.active = true;
    ad.angle.end = end;
    ad.angle.has_start = false;
    ad.angle.relative_node.reset(new Node(relative_node));
}

/**
 * Add a rotation animation, specifying the start and end angle in degrees.
 */
void AnimationType::
add_rotation(double start, double end) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.angle.active = true;
    ad.angle.start = start;
    ad.angle.end = end;
    ad.angle.has_start = true;
    if (ad.angle.relative_node) {
        ad.angle.relative_node.reset(nullptr);
    }
}

/**
 * Add a rotation animation relative to a Node, specifying the start and end
 * angle in degrees and a Node.
 */
void AnimationType::
add_rotation(double start, double end, Node relative_node) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.angle.active = true;
    ad.angle.end = start;
    ad.angle.end = end;
    ad.angle.has_start = true;
    ad.angle.relative_node.reset(new Node(relative_node));
}

/**
 * Add a position animation for the rotation center, only specifying the end
 * point.
 */
void AnimationType::
add_rotation_center(Vec2 end) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.center_pos.active = true;
    ad.center_pos.end = end;
    ad.center_pos.has_start = false;
}

/**
 * Add a position animation for the rotation center, specifying the end and
 * start point.
 */
void AnimationType::
add_rotation_center(Vec2 start, Vec2 end) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.center_pos.active = true;
    ad.center_pos.start = start;
    ad.center_pos.end = end;
    ad.center_pos.has_start = true;
}

/**
 * Add a depth animation, only specifying the end depth.
 */
void AnimationType::
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
void AnimationType::
add_depth(int end, Node relative_node) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.depth.active = true;
    ad.depth.end = end;
    ad.depth.has_start = false;
    ad.depth.relative_node.reset(new Node(relative_node));
}

/**
 * Add a depth animation, specifying the start and end depth.
 */
void AnimationType::
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
void AnimationType::
add_depth(int start, int end, Node relative_node) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.depth.active = true;
    ad.depth.start = start;
    ad.depth.end = end;
    ad.depth.has_start = true;
    ad.depth.relative_node.reset(new Node(relative_node));
}


// AnimationType - Control

/**
 * Reset command that needs to be overridden in subclasses.
 */
void AnimationType::
reset() {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.playback_pos = -1.0;
    if (ad.pos.active && !ad.pos.has_start) {
        if (ad.pos.relative_node) {
            ad.pos.start = ad.node->get_pos(*ad.pos.relative_node);
        }
        else {
            ad.pos.start = ad.node->get_pos();
        }
    }

    if (ad.center_pos.active && !ad.center_pos.has_start) {
        ad.pos.start = ad.node->get_rotation_center();
    }

    if (ad.scale.active && !ad.scale.has_start) {
        if (ad.scale.relative_node) {
            ad.scale.start = ad.node->get_scale(*ad.scale.relative_node);
        }
        else {
            ad.scale.start = ad.node->get_scale();
        }
    }

    if (ad.angle.active && !ad.angle.has_start) {
        if (ad.angle.relative_node) {
            ad.angle.start = ad.node->get_angle(*ad.angle.relative_node);
        }
        else {
            ad.angle.start = ad.node->get_angle();
        }
    }

    if (ad.depth.active && !ad.depth.has_start) {
        if (ad.depth.relative_node) {
            ad.depth.start = ad.node->get_depth(*ad.depth.relative_node);
        }
        else {
            ad.depth.start = ad.node->get_depth();
        }
    }
}

/**
 * Returns the current playback position in seconds.
 */
double AnimationType::
get_playback_pos() {
    AnimationData& ad = _get_animation_data(_animation_id);
    return ad.playback_pos;
}

/**
 *
 */
char AnimationType::
active_animations() {
    return 0;
}

/**
 *
 */
int AnimationType::
node_id() {
    AnimationData& ad = _get_animation_data(_animation_id);
    return ad.node->get_id();
}

/**
 * Helper method to retrieve the AnimationData for the specified
 * ``animation_id``.
 */
AnimationData& AnimationType::
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
void Interval::
set_duration(const double d) {
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.duration = d;
}

/**
 * Reset to initial state. Updates start states, for the active animation types,
 * where none have been explicitly specified.
 */
void Interval::
reset() {
    AnimationType::reset();
}

/**
 * Returns -1.0 if the Interval is not finished, otherwise returns the amount
 * of ``dt`` seconds remaining after the Interval was complete. Returns -2.0
 * if executing the Interval would cause a conflict.
 */
double Interval::
step(const double dt, ActiveAnimationMap& aam) {
    AnimationData& ad = _get_animation_data(_animation_id);

    int node_id = ad.node->get_id();
    char active_anim = active_animations();
    auto search = aam.find(node_id);
    if (search != aam.end()) {
        if ((aam[node_id] & active_anim) > 0) {
            return -2.0;
        }
    }
    else {
        aam[node_id] = 0;
    }
    aam[node_id] = aam[node_id] | active_anim;

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
void Interval::
_update(const double prog) {
    AnimationData& ad = _get_animation_data(_animation_id);
    // position
    if (ad.pos.active) {
		Vec2 v = Vec2(
				(ad.pos.end - ad.pos.start) * prog + ad.pos.start);
        if (ad.pos.relative_node) {
            ad.node->set_pos(*ad.pos.relative_node, v);
        }
        else {
            ad.node->set_pos(v);
        }
    }

    // rotation_center
    if (ad.center_pos.active) {
		Vec2 v = Vec2(
				(ad.center_pos.end - ad.center_pos.start) * prog
				+ ad.center_pos.start);
        ad.node->set_rotation_center(v);
    }

    // scale
    if (ad.scale.active) {
		Scale s = (ad.scale.end - ad.scale.start)
			* prog + ad.scale.start;
        if (ad.scale.relative_node) {
            ad.node->set_scale(*ad.scale.relative_node, s);
        }
        else {
            ad.node->set_scale(s);
        }
    }

    // rotation
    if (ad.angle.active) {
        if (ad.angle.relative_node) {
            ad.node->set_angle(
                *ad.angle.relative_node,
                (ad.angle.end - ad.angle.start) * prog + ad.angle.start);
        }
        else {
            ad.node->set_angle(
                (ad.angle.end - ad.angle.start) * prog + ad.angle.start);
        }
    }

    // depth
    if (ad.depth.active) {
        const int target_depth = static_cast<int>(
            (ad.depth.end - ad.depth.start) * prog + ad.depth.start + 0.5);
        if (ad.depth.relative_node) {
            ad.node->set_depth(*ad.depth.relative_node, target_depth);
        }
        else {
            ad.node->set_depth(target_depth);
        }
    }
}

/**
 *
 */
std::unique_ptr<AnimationBase> Interval::
get_copy() {
    std::unique_ptr<Interval> ptr;
    ptr.reset(new Interval());
    AnimationData& ad = _get_animation_data(_animation_id);
    AnimationData& ptr_ad = ptr->_get_animation_data(ptr->_animation_id);

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
    ptr_ad.node.reset(new Node(*ad.node));
    ptr_ad.blend = ad.blend;
    ptr_ad.pos.start = ad.pos.start;
    ptr_ad.pos.end = ad.pos.end;
    if (ad.pos.relative_node) {
		ptr_ad.pos.relative_node.reset(new Node(
					*ad.pos.relative_node));
	}
    ptr_ad.pos.active = ad.pos.active;
    ptr_ad.pos.has_start = ad.pos.has_start;
    ptr_ad.center_pos.start = ad.center_pos.start;
    ptr_ad.center_pos.end = ad.center_pos.end;
    ptr_ad.center_pos.active = ad.center_pos.active;
    ptr_ad.center_pos.has_start = ad.center_pos.has_start;
    ptr_ad.scale.start = ad.scale.start;
    ptr_ad.scale.end = ad.scale.end;
    if (ad.scale.relative_node) {
		ptr_ad.scale.relative_node.reset(new Node(
					*ad.scale.relative_node));
	}
    ptr_ad.scale.active = ad.scale.active;
    ptr_ad.scale.has_start = ad.scale.has_start;
    ptr_ad.angle.start = ad.angle.start;
    ptr_ad.angle.end = ad.angle.end;
    if (ad.angle.relative_node) {
	    ptr_ad.angle.relative_node.reset(new Node(
	    			*ad.angle.relative_node));
	}
    ptr_ad.angle.active = ad.angle.active;
    ptr_ad.angle.has_start = ad.angle.has_start;
    ptr_ad.depth.start = ad.depth.start;
    ptr_ad.depth.end = ad.depth.end;
    if (ad.depth.relative_node) {
    	ptr_ad.depth.relative_node.reset(new Node(
    				*ad.depth.relative_node));
    }
    ptr_ad.depth.active = ad.depth.active;
    ptr_ad.depth.has_start = ad.depth.has_start;
    return ptr;
}

/**
 *
 */
char Interval::
active_animations() {
    char active_anim = 0;
    AnimationData& ad = _get_animation_data(_animation_id);
    if (ad.pos.active) {
        active_anim = active_anim | 1;
    }
    if (ad.center_pos.active) {
        active_anim = active_anim | 2;
    }
    if (ad.scale.active) {
        active_anim = active_anim | 4;
    }
    if (ad.angle.active) {
        active_anim = active_anim | 8;
    }
    if (ad.depth.active) {
        active_anim = active_anim | 16;
    }
    return active_anim;
}


// Animation

/**
 *
 */
void Animation::
set_pos_speed(const double s) {
    _get_animation_data(_animation_id).pos_speed = s;
}

/**
 *
 */
void Animation::
set_scale_speed(const double s) {
    _get_animation_data(_animation_id).scale_speed = s;
}

/**
 *
 */
void Animation::
set_rotation_speed(const double s) {
    _get_animation_data(_animation_id).rotation_speed = s;
}

/**
 *
 */
void Animation::
set_rotation_center_speed(const double s) {
    _get_animation_data(_animation_id).rotation_center_speed = s;
}

/**
 *
 */
void Animation::
set_depth_speed(const double s) {
    _get_animation_data(_animation_id).depth_speed = s;
}

/**
 * Reset to initial state. Updates start states, for the active animation types,
 * where none have been explicitly specified and translates speed into duration.
 */
void Animation::
reset() {
    AnimationType::reset();
    AnimationData& ad = _get_animation_data(_animation_id);
    ad.duration = -1.0;

    if (ad.pos.active) {
        if (ad.pos_speed < 0.0) {
            throw std::logic_error("Position animation specified without "
                                   "speed");
        }
        double tmp_d = (ad.pos.end - ad.pos.start).length() / ad.pos_speed;
        ad.dur_pos = tmp_d;
        if (tmp_d > ad.duration) {
            ad.duration = tmp_d;
        }
    }

    if (ad.center_pos.active) {
        if (ad.rotation_center_speed < 0.0) {
            throw std::logic_error("Rotation center animation specified "
								   "without speed");
        }
        double tmp_d = (ad.center_pos.end - ad.center_pos.start).length()
                       / ad.rotation_center_speed;
        ad.dur_center_pos = tmp_d;
        if (tmp_d > ad.duration) {
            ad.duration = tmp_d;
        }
    }

    if (ad.scale.active) {
        if (ad.scale_speed < 0.0) {
            throw std::logic_error("Scale animation specified without "
                                   "speed");
        }
        ad.dur_scalex = std::abs(
            ad.scale.end.sx - ad.scale.start.sx) / ad.scale_speed;

        ad.dur_scaley = std::abs(
            ad.scale.end.sy - ad.scale.start.sy) / ad.scale_speed;
        double tmp_d = std::max(ad.dur_scalex, ad.dur_scaley);
        if (tmp_d > ad.duration) {
            ad.duration = tmp_d;
        }
    }

    if (ad.angle.active) {
        if (ad.rotation_speed == 0.0) {
            throw std::logic_error("Rotation animation specified without "
                                   "speed");
        }
        ad.dur_angle = std::abs(
            ad.angle.end - ad.angle.start) / ad.rotation_speed;
        if (ad.dur_angle > ad.duration) {
            ad.duration = ad.dur_angle;
        }
    }

    if (ad.depth.active) {
        if (ad.depth_speed < 0.0) {
            throw std::logic_error("Depth animation specified without "
                                   "speed");
        }
        ad.dur_depth = std::abs(
            ad.depth.end - ad.depth.start) / ad.depth_speed;
        if (ad.dur_depth > ad.duration) {
            ad.duration = ad.dur_depth;
        }
    }

    if (ad.duration == -1.0) {
        throw std::logic_error("Tried to reset an animation w/o any modifier "
                               "active");
    }
}

/**
 * Returns -1.0 if the Animation is not finished, otherwise returns the amount
 * of ``dt`` seconds remaining after the Animation was complete. Returns -2.0
 * if executing the Animation would cause a conflict.
 */
double Animation::
step(const double dt, ActiveAnimationMap& aam) {

    AnimationData& ad = _get_animation_data(_animation_id);
    if (ad.playback_pos == -1.0) {
        ad.playback_pos = 0.0;
    }
    // else if (ad.playback_pos >= ad.duration) {
    //     return ad.playback_pos - ad.duration;
    // }
    int node_id = ad.node->get_id();
    char active_anim = active_animations();
    auto search = aam.find(node_id);
    if (search != aam.end()) {
        if ((aam[node_id] & active_anim) > 0) {
            return -2.0;
        }
    }
    else {
        aam[node_id] = 0;
    }
    aam[node_id] = aam[node_id] | active_anim;

    ad.playback_pos += dt;

    // position
    if (ad.pos.active) {
        Vec2 p = (ad.pos.relative_node)
                        ? ad.node->get_pos(*ad.pos.relative_node)
                        : ad.node->get_pos();
        if (p != ad.pos.end) {
            if (ad.playback_pos >= ad.dur_pos) {
                if (ad.pos.relative_node) {
                    ad.node->set_pos(*ad.pos.relative_node, ad.pos.end);
                }
                else {
                    ad.node->set_pos(ad.pos.end);
                }
            }
            else {
                double prog = lerp(ad.playback_pos, ad.dur_pos, ad.blend);
				Vec2 v = Vec2((ad.pos.end - ad.pos.start)
						* prog + ad.pos.start);
                if (ad.pos.relative_node) {
                    ad.node->set_pos(*ad.pos.relative_node, v);
                }
                else {
                    ad.node->set_pos(v);
                }
            }
        }
    }

    // rotation_center
    if (ad.center_pos.active) {
        Vec2 p = ad.node->get_rotation_center();
        if (p != ad.center_pos.end) {
            if (ad.playback_pos >= ad.dur_center_pos) {
                ad.node->set_rotation_center(ad.center_pos.end);
            }
            else {
                double prog = lerp(ad.playback_pos, ad.dur_center_pos,
                    ad.blend);
				Vec2 v = Vec2(
						(ad.center_pos.end - ad.center_pos.start) * prog
						+ ad.center_pos.start);
                ad.node->set_rotation_center(v);
            }
        }
    }

    // scale
    if (ad.scale.active) {
        Scale s = (ad.scale.relative_node)
                            ? ad.node->get_scale(*ad.scale.relative_node)
                            : ad.node->get_scale();
        Scale new_scale = s;
        if (s.sx != ad.scale.end.sx) {
            if (ad.playback_pos >= ad.dur_scalex) {
                new_scale.sx = ad.scale.end.sx;
            }
            else {
                double prog = lerp(ad.playback_pos, ad.dur_scalex, ad.blend);
                new_scale.sx = (ad.scale.end.sx - ad.scale.start.sx) * prog
                    + ad.scale.start.sx;
            }
        }
        if (s.sy != ad.scale.end.sy) {
            if (ad.playback_pos >= ad.dur_scaley) {
                new_scale.sy = ad.scale.end.sy;
            }
            else {
                double prog = lerp(ad.playback_pos, ad.dur_scaley, ad.blend);
                new_scale.sy = (ad.scale.end.sy - ad.scale.start.sy) * prog
                    + ad.scale.start.sy;
            }
        }

        if (s != new_scale) {
            if (ad.scale.relative_node) {
                ad.node->set_scale(*ad.scale.relative_node, new_scale);
            }
            else {
                ad.node->set_scale(new_scale);
            }
        }
    }

    // rotation
    if (ad.angle.active) {
        double a = (ad.angle.relative_node)
                    ? ad.node->get_angle(*ad.angle.relative_node)
                    : ad.node->get_angle();
        if (a != ad.angle.end) {
            if (ad.playback_pos >= ad.dur_angle) {
                if (ad.angle.relative_node) {
                    ad.node->set_angle(*ad.angle.relative_node, ad.angle.end);
                }
                else {
                    ad.node->set_angle(ad.angle.end);
                }
            }
            else {
                double prog = lerp(ad.playback_pos, ad.dur_angle, ad.blend);
                if (ad.angle.relative_node) {
                    ad.node->set_angle(
                        *ad.angle.relative_node,
                        (ad.angle.end - ad.angle.start) * prog + ad.angle.start
                    );
                }
                else {
                    ad.node->set_angle(
                        (ad.angle.end - ad.angle.start) * prog + ad.angle.start
                    );
                }
            }
        }
    }

    // depth
    if (ad.depth.active) {
        int depth = (ad.depth.relative_node)
                    ? ad.node->get_depth(*ad.depth.relative_node)
                    : ad.node->get_depth();
        if (depth != ad.depth.end) {
            if (ad.playback_pos >= ad.dur_depth) {
                if (ad.depth.relative_node) {
                    ad.node->set_depth(*ad.depth.relative_node, ad.depth.end);
                }
                else {
                    ad.node->set_depth(ad.depth.end);
                }
            }
            else {
                double prog = lerp(ad.playback_pos, ad.dur_depth, ad.blend);
                const int target_depth = static_cast<int>(
                    (ad.depth.end - ad.depth.start) * prog + ad.depth.start
                    + 0.5);
                if (ad.depth.relative_node) {
                    ad.node->set_depth(*ad.depth.relative_node, target_depth);
                }
                else {
                    ad.node->set_depth(target_depth);
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
std::unique_ptr<AnimationBase> Animation::
get_copy() {
    std::unique_ptr<Animation> ptr;
    ptr.reset(new Animation());
    AnimationData& ad = _get_animation_data(_animation_id);
    AnimationData& ptr_ad = ptr->_get_animation_data(ptr->_animation_id);

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
    ptr_ad.node.reset(new Node(*ad.node));
    ptr_ad.blend = ad.blend;
    ptr_ad.pos.start = ad.pos.start;
    ptr_ad.pos.end = ad.pos.end;
    if (ad.pos.relative_node) {
		ptr_ad.pos.relative_node.reset(new Node(
					*ad.pos.relative_node));
	}
    ptr_ad.pos.active = ad.pos.active;
    ptr_ad.pos.has_start = ad.pos.has_start;
    ptr_ad.center_pos.start = ad.center_pos.start;
    ptr_ad.center_pos.end = ad.center_pos.end;
    ptr_ad.center_pos.active = ad.center_pos.active;
    ptr_ad.center_pos.has_start = ad.center_pos.has_start;
    ptr_ad.scale.start = ad.scale.start;
    ptr_ad.scale.end = ad.scale.end;
    if (ad.scale.relative_node) {
		ptr_ad.scale.relative_node.reset(new Node(
					*ad.scale.relative_node));
	}
    ptr_ad.scale.active = ad.scale.active;
    ptr_ad.scale.has_start = ad.scale.has_start;
    ptr_ad.angle.start = ad.angle.start;
    ptr_ad.angle.end = ad.angle.end;
    if (ad.angle.relative_node) {
	    ptr_ad.angle.relative_node.reset(new Node(
	    			*ad.angle.relative_node));
	}
    ptr_ad.angle.active = ad.angle.active;
    ptr_ad.angle.has_start = ad.angle.has_start;
    ptr_ad.depth.start = ad.depth.start;
    ptr_ad.depth.end = ad.depth.end;
    if (ad.depth.relative_node) {
    	ptr_ad.depth.relative_node.reset(new Node(
    				*ad.depth.relative_node));
    }
    ptr_ad.depth.active = ad.depth.active;
    ptr_ad.depth.has_start = ad.depth.has_start;
    return ptr;
}

/**
 *
 */
char Animation::
active_animations() {
    char active_anim = 0;
    AnimationData& ad = _get_animation_data(_animation_id);
    if (ad.pos.active && ad.playback_pos < ad.dur_pos) {
        active_anim = active_anim | 1;
    }
    if (ad.center_pos.active && ad.playback_pos < ad.dur_center_pos) {
        active_anim = active_anim | 2;
    }
    if (ad.scale.active && ad.playback_pos < ad.dur_scalex) {
        active_anim = active_anim | 4;
    }
    if (ad.scale.active && ad.playback_pos < ad.dur_scaley) {
        active_anim = active_anim | 4;
    }
    if (ad.angle.active && ad.playback_pos < ad.dur_angle) {
        active_anim = active_anim | 8;
    }
    if (ad.depth.active && ad.playback_pos < ad.dur_depth) {
        active_anim = active_anim | 16;
    }
    return active_anim;
}

// Sequence

/**
 *
 */
void Sequence::
append(std::unique_ptr<AnimationBase>& a) {
    _v.push_back(a->get_copy());

    // Make sure no sequences with loop = true are appended.
    _v[_v.size() - 1]->loop(false);
}

/**
 *
 */
void Sequence::
reset() {
    if (!_v.size()) {
        throw std::runtime_error("Tried to reset empty Sequence.");
    }
    _active = 0;
    _v[0]->reset();
}

/**
 *
 */
double Sequence::
step(const double dt, ActiveAnimationMap& aam) {
    if (!_v.size()) {
        throw std::runtime_error("Tried to step empty Sequence.");
    }
    double rdt = 1.0;
    while (rdt >= 0.0) {
        ActiveAnimationMap tmp = aam;
        rdt = _v[_active]->step(dt, aam);
        if (rdt < 0.0) {
            break;
        }

        ++_active;
        if (_active == _v.size()) {
            if (_loop) {
                _active = 0;
            }
            else {
                return rdt;
            }
        }

        _v[_active]->reset();
        aam = tmp;
    }
    return rdt;
}

/**
 *
 */
void Sequence::
loop(const bool l) {
    _loop = l;
}

/**
 *
 */
std::unique_ptr<AnimationBase> Sequence::
get_copy() {
    std::unique_ptr<AnimationBase> ptr;
    ptr.reset(new Sequence());
    Sequence& sq = static_cast<Sequence&>(*ptr);
    for (auto it = _v.begin(); it != _v.end(); ++it) {
        sq.append(*it);
    }
    return ptr;
}

// AnimationManager

/**
 * Returns the id of a new and empty Interval.
 */
int AnimationManager::
new_interval() {
    ++_max_anim;
    _anims[_max_anim] = std::unique_ptr<AnimationBase>(new Interval());
    _anim_status[_max_anim] = 0;
    return _max_anim;
}

/**
 * Returns the id of a new and empty Animation.
 */
int AnimationManager::
new_animation() {
    ++_max_anim;
    _anims[_max_anim] = std::unique_ptr<AnimationBase>(new Animation());
    _anim_status[_max_anim] = 0;
    return _max_anim;
}

/**
 * Returns the id of a new and empty Sequence.
 */
int AnimationManager::
new_sequence() {
    ++_max_anim;
    _anims[_max_anim] = std::unique_ptr<AnimationBase>(new Sequence());
    _anim_status[_max_anim] = 0;
    return _max_anim;
}

/**
 * Return a Interval reference for the specified id.
 */
Interval& AnimationManager::
get_interval(const int i_id) {
    if (_anims.find(i_id) == _anims.end()) {
        throw std::range_error("Specified id is not an active Interval");
    }
    return (Interval&) *_anims[i_id];
}

/**
 * Return a Animation reference for the specified id.
 */
Animation& AnimationManager::
get_animation(const int a_id) {
    if (_anims.find(a_id) == _anims.end()) {
        throw std::range_error("Specified id is not an active Animation");
    }
    return (Animation&) *_anims[a_id];
}

/**
 * Return a Sequence reference for the specified id.
 */
Sequence& AnimationManager::
get_sequence(const int s_id) {
    if (_anims.find(s_id) == _anims.end()) {
        throw std::range_error("Specified id is not an active Sequence");
    }
    return (Sequence&) *_anims[s_id];
}

/**
 * Return a AnimationBase unique_ptr reference for the specified id.
 */
std::unique_ptr<AnimationBase>& AnimationManager::
get_animation_base_ptr(const int i_id) {
    if (_anims.find(i_id) == _anims.end()) {
        throw std::range_error("Specified id is not an active Interval");
    }
    return _anims[i_id];
}

/**
 * Removes the specified Interval.
 */
void AnimationManager::
remove_interval(const int i_id) {
    if (_anims.find(i_id) == _anims.end()) {
        throw std::range_error("Specified id is not an active Interval");
    }
    _anim_status.erase(i_id);
    _anims.erase(i_id);
}

/**
 * Removes the specified Animation.
 */void AnimationManager::
remove_animation(const int a_id) {
    if (_anims.find(a_id) == _anims.end()) {
        throw std::range_error("Specified id is not an active Animation");
    }
    _anim_status.erase(a_id);
    _anims.erase(a_id);
}

/**
 * Removes the specified Sequence.
 */
void AnimationManager::
remove_sequence(const int s_id) {
    if (_anims.find(s_id) == _anims.end()) {
        throw std::range_error("Specified id is not an active Sequence");
    }
    _anim_status.erase(s_id);
    _anims.erase(s_id);
}

/**
 * Play Interval from beginning.
 */
void AnimationManager::
play_interval(const int i_id) {
    auto search = _anim_status.find(i_id);
    if (search != _anim_status.end()) {
        _anim_status[i_id] = 2;
    }
    else {
        throw std::range_error("Specified id is not an active Interval");
    }
}

/**
 * Play Animation from beginning.
 */
void AnimationManager::
play_animation(const int a_id) {
    auto search = _anim_status.find(a_id);
    if (search != _anim_status.end()) {
        _anim_status[a_id] = 2;
    }
    else {
        throw std::range_error("Specified id is not an active Animation");
    }
}

/**
 * Play Sequence from beginning.
 */
void AnimationManager::
play_sequence(const int s_id) {
    auto search = _anim_status.find(s_id);
    if (search != _anim_status.end()) {
        _anim_status[s_id] = 2;
    }
    else {
        throw std::range_error("Specified id is not an active Sequence");
    }
}

/**
 * Pause Interval.
 */
void AnimationManager::
pause_interval(const int i_id) {
    auto search = _anim_status.find(i_id);
    if (search != _anim_status.end()) {
        if (_anim_status[i_id] != 3) {
            throw std::logic_error("Unable to pause Interval, not playing");
        }
        _anim_status[i_id] = 1;
    }
    else {
        throw std::range_error("Specified id is not an active Interval");
    }
}

/**
 * Pause Animation.
 */
void AnimationManager::
pause_animation(const int a_id) {
    auto search = _anim_status.find(a_id);
    if (search != _anim_status.end()) {
        if (_anim_status[a_id] != 3) {
            throw std::logic_error("Unable to pause Animation, not playing");
        }
        _anim_status[a_id] = 1;
    }
    else {
        throw std::range_error("Specified id is not an active Animation");
    }
}

/**
 * Pause Sequence.
 */
void AnimationManager::
pause_sequence(const int s_id) {
    auto search = _anim_status.find(s_id);
    if (search != _anim_status.end()) {
        if (_anim_status[s_id] != 3) {
            throw std::logic_error("Unable to pause Sequence, not playing");
        }
        _anim_status[s_id] = 1;
    }
    else {
        throw std::range_error("Specified id is not an active Sequence");
    }
}

/**
 * Resume Interval.
 */
void AnimationManager::
resume_interval(const int i_id) {
    auto search = _anim_status.find(i_id);
    if (search != _anim_status.end()) {
        if (_anim_status[i_id] != 1) {
            throw std::logic_error("Unable to resume Interval, not paused");
        }
        _anim_status[i_id] = 3;
    }
    else {
        throw std::range_error("Specified id is not an active Interval");
    }
}

/**
 * Resume Animation.
 */
void AnimationManager::
resume_animation(const int a_id) {
    auto search = _anim_status.find(a_id);
    if (search != _anim_status.end()) {
        if (_anim_status[a_id] != 1) {
            throw std::logic_error("Unable to resume Animation, not paused");
        }
        _anim_status[a_id] = 3;
    }
    else {
        throw std::range_error("Specified id is not an active Animation");
    }
}

/**
 * Resume Sequence.
 */
void AnimationManager::
resume_sequence(const int s_id) {
    auto search = _anim_status.find(s_id);
    if (search != _anim_status.end()) {
        if (_anim_status[s_id] != 1) {
            throw std::logic_error("Unable to resume Sequence, not paused");
        }
        _anim_status[s_id] = 3;
    }
    else {
        throw std::range_error("Specified id is not an active Sequence");
    }
}

/**
 * Stop Interval.
 */
void AnimationManager::
stop_interval(const int i_id) {
    auto search = _anim_status.find(i_id);
    if (search != _anim_status.end()) {
        _anim_status[i_id] = 0;
    }
    else {
        throw std::range_error("Specified id is not an active Interval");
    }
}

/**
 * Stop Animation.
 */
void AnimationManager::
stop_animation(const int a_id) {
    auto search = _anim_status.find(a_id);
    if (search != _anim_status.end()) {
        _anim_status[a_id] = 0;
    }
    else {
        throw std::range_error("Specified id is not an active Animation");
    }
}

/**
 * Stop Sequence.
 */
void AnimationManager::
stop_sequence(const int s_id) {
    auto search = _anim_status.find(s_id);
    if (search != _anim_status.end()) {
        _anim_status[s_id] = 0;
    }
    else {
        throw std::range_error("Specified id is not an active Sequence");
    }
}

/**
 * Return playback status of an Interval.
 */
char AnimationManager::
get_interval_status(const int i_id) {
	auto search = _anim_status.find(i_id);
	if (search == _anim_status.end()) {
		throw std::range_error("Specified id is not an active Interval/"
							   "Animation.");
	}
	return _anim_status[i_id];
}

/**
 * Return playback status of an Animation.
 */
char AnimationManager::
get_animation_status(const int a_id) {
	auto search = _anim_status.find(a_id);
	if (search == _anim_status.end()) {
		throw std::range_error("Specified id is not an active Animation.");
	}
	return get_interval_status(a_id);
}

/**
 * Return playback status of an Interval.
 */
char AnimationManager::
get_sequence_status(const int s_id) {
	auto search = _anim_status.find(s_id);
	if (search == _anim_status.end()) {
		throw std::range_error("Specified id is not an active Sequence.");
	}
	return _anim_status[s_id];
}

/*
 * Append to Sequence by id.
 */
void AnimationManager::
append(const int s_id, const int a_id) {
    if (s_id == a_id) {
        throw std::runtime_error("Cannot append sequence to itself.");
    }
    get_sequence(s_id).append(get_animation_base_ptr(a_id));
}

/**
 * Advance animation playback by ``dt`` seconds.
 */
void AnimationManager::
animate(const double dt) {
	_aam.clear();
	// Animations/Intervals
	for (auto it = _anims.begin(); it != _anims.end(); ++it) {
		if (_anim_status[it->first] < 2) {
			continue;
		}
		else if (_anim_status[it->first] == 2) {
			it->second->reset();
			_anim_status[it->first] = 3;
		}
		const double r = it->second->step(dt, _aam);
		if (r == -2.0) {
			_anim_status[it->first] = 4;
			// TODO: Log conflict
			continue;
		}
        else if (r >= 0.0) {
			_anim_status[it->first] = 0;
        }
	}
}


}  // namespace animation
}  // namespace foolysh
