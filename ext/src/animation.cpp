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


// AnimationType - Setup

/**
 * Set the node, the animation will apply to.
 */
void animation::AnimationType::
set_node(scenegraph::Node n) {
    node = n;
}

/**
 * Set the BlendType, used for the animation.
 */
void animation::AnimationType::
set_blend(BlendType b) {
    blend = b;
}

/**
 * Add a position animation, only specifying the end point.
 */
void animation::AnimationType::
add_pos(tools::Vector2 end) {
    pos.active = true;
    pos.end = end;
    pos.has_start = false;
    if (pos.relative_node) {
        pos.relative_node.reset(nullptr);
    }
}

/**
 * Add a position animation relative to a Node, specifying the end point and a
 * Node.
 */
void animation::AnimationType::
add_pos(tools::Vector2 end, scenegraph::Node relative_node) {
    pos.active = true;
    pos.end = end;
    pos.has_start = false;
    pos.relative_node.reset(scenegraph::Node(relative_node));
}

/**
 * Add a position animation, specifying start and end points.
 */
void animation::AnimationType::
add_pos(tools::Vector2 start, tools::Vector2 end) {
    pos.active = true;
    pos.start = start;
    pos.end = end;
    pos.has_start = true;
    if (pos.relative_node) {
        pos.relative_node.reset(nullptr);
    }
}

/**
 * Add a position animation relative to a Node, specifying the start, end point
 * and a Node.
 */
void animation::AnimationType::
add_pos(tools::Vector2 start, tools::Vector2 end,
        scenegraph::Node relative_node) {
    pos.active = true;
    pos.start = start;
    pos.end = end;
    pos.has_start = true;
    pos.relative_node.reset(scenegraph::Node(relative_node));
}

/**
 * Add a scale animation, only specifying the end scale.
 */
void animation::AnimationType::
add_scale(scenegraph::Scale end) {
    scale.active = true;
    scale.end = end;
    scale.has_start = false;
    if (scale.relative_node) {
        scale.relative_node.reset(nullptr);
    }
}

/**
 * Add a scale animation relative to a Node, specifying the end scale and a
 * Node.
 */
void animation::AnimationType::
add_scale(scenegraph::Scale end, scenegraph::Node relative_node) {
    scale.active = true;
    scale.end = end;
    scale.has_start = false;
    scale.relative_node.reset(new scenegraph::Node(relative_node));
}

/**
 * Add a scale animation, specifying the start and end scale.
 */
void animation::AnimationType::
add_scale(scenegraph::Scale start, scenegraph::Scale end) {
    scale.active = true;
    scale.start = start;
    scale.end = end;
    scale.has_start = true;
    if (scale.relative_node) {
        scale.relative_node.reset(nullptr);
    }
}

/**
 * Add a scale animation relative to a Node, specifying the start and end scale
 * and a Node.
 */
void animation::AnimationType::
add_scale(scenegraph::Scale start, scenegraph::Scale end,
          scenegraph::Node relative_node) {
    scale.active = true;
    scale.start = start;
    scale.end = end;
    scale.has_start = false;
    scale.relative_node.reset(new scenegraph::Node(relative_node));
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
    angle.active = true;
    angle.end = start;
    angle.end = end;
    angle.has_start = true;
    angle.relative_node.reset(new scenegraph::Node(relative_node));
}

/**
 * Add a position animation for the rotation center, only specifying the end
 * point.
 */
void animation::AnimationType::
add_rotation_center(tools::Vector2 end) {
    center_pos.active = true;
    center_pos.end = end;
    center_pos.has_start = false;
}

/**
 * Add a position animation for the rotation center, specifying the end and
 * start point.
 */
void animation::AnimationType::
add_rotation_center(tools::Vector2 start, tools::Vector2 end) {
    center_pos.active = true;
    center_pos.start = start;
    center_pos.end = end;
    center_pos.has_start = true;
}

/**
 * Add a depth animation, only specifying the end depth.
 */
void animation::AnimationType::
add_depth(int end) {
    depth.active = true;
    depth.end = end;
    depth.has_start = false;
    if (depth.relative_node) {
        depth.relative_node.reset(nullptr);
    }
}

/**
 * Add a depth animation relative to a Node, specifying the end depth and a
 * Node.
 */
void animation::AnimationType::
add_depth(int end, scenegraph::Node relative_node) {
    depth.active = true;
    depth.end = end;
    depth.has_start = false;
    depth.relative_node.reset(new scenegraph::Node(relative_node));
}

/**
 * Add a depth animation, specifying the start and end depth.
 */
void animation::AnimationType::
add_depth(int start, int end) {
    depth.active = true;
    depth.start = start;
    depth.end = end;
    depth.has_start = true;
    if (depth.relative_node) {
        depth.relative_node.reset(nullptr);
    }
}

/**
 * Add a depth animation relative to a Node, specifying the start and end depth
 * and a Node.
 */
void animation::AnimationType::
add_depth(int start, int end, scenegraph::Node relative_node) {
    depth.active = true;
    depth.start = start;
    depth.end = end;
    depth.has_start = true;
    depth.relative_node.reset(new scenegraph::Node(relative_node));
}
