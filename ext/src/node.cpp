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

/**
 * Node implementation
 */

#include "node.hpp"
#include "common.hpp"

#include <cmath>
#include <stdexcept>

namespace foolysh {
namespace scene {

using foolysh::tools::SmallList;
using foolysh::tools::Vec2;

/* SceneGraphDataHandler */

/**
 * Get an empty index for a new Node.
 **/
size_t SceneGraphDataHandler::
get_empty() {
    if (free_vec.size()) {
        const size_t node_id = free_vec.back();
        free_vec.pop_back();
        flag_vec[node_id] = 0;
        return node_id;
    }
    pos_x.push_back(0.0);
    pos_y.push_back(0.0);
    r_pos_x.push_back(0.0);
    r_pos_y.push_back(0.0);
    scale_x.push_back(0.0);
    scale_y.push_back(0.0);
    r_scale_x.push_back(0.0);
    r_scale_y.push_back(0.0);
    size_x.push_back(0.0);
    size_y.push_back(0.0);
    rotation_center_x.push_back(0.0);
    rotation_center_y.push_back(0.0);
    angle_vec.push_back(0.0);
    r_angle_vec.push_back(0.0);
    depth_vec.push_back(0);
    r_depth_vec.push_back(0);
    flag_vec.push_back(0);
    origin_vec.push_back(TOP_LEFT);
    parent_vec.push_back(0);
    ref_vec.push_back(1);
    return pos_x.size() - 1;
}

/**
 * Erase a Node reference.
 **/
void SceneGraphDataHandler::
erase(const size_t node_id) {
    --ref_vec[node_id];
    if (ref_vec[node_id]) {
        return;
    }
    flag_vec[node_id] = flag_vec[node_id] | FREE;
    free_vec.push_back(node_id);
}

/**
 * Reserve memory. Useful when the number of Nodes needed is known.
 **/
void SceneGraphDataHandler::
reserve(const size_t size) {
    pos_x.reserve(size);
    pos_y.reserve(size);
    r_pos_x.reserve(size);
    r_pos_y.reserve(size);
    scale_x.reserve(size);
    scale_y.reserve(size);
    r_scale_x.reserve(size);
    r_scale_y.reserve(size);
    size_x.reserve(size);
    size_y.reserve(size);
    rotation_center_x.reserve(size);
    rotation_center_y.reserve(size);
    angle_vec.reserve(size);
    r_angle_vec.reserve(size);
    depth_vec.reserve(size);
    r_depth_vec.reserve(size);
    flag_vec.reserve(size);
    origin_vec.reserve(size);
    parent_vec.reserve(size);
    ref_vec.reserve(size);
}

/* Node */

/**
 * Construct new Node.
 **/
Node::
Node(SceneGraphDataHandler& sgdh) : sgdh(sgdh) {
    node_id = sgdh.get_empty();
    sgdh.pos_x[node_id] = 0.0;
    sgdh.pos_y[node_id] = 0.0;
    sgdh.r_pos_x[node_id] = 0.0;
    sgdh.r_pos_y[node_id] = 0.0;
    sgdh.scale_x[node_id] = 1.0;
    sgdh.scale_y[node_id] = 1.0;
    sgdh.size_x[node_id] = 0.0;
    sgdh.size_y[node_id] = 0.0;
    sgdh.rotation_center_x[node_id] = 0.0;
    sgdh.rotation_center_y[node_id] = 0.0;
    sgdh.angle_vec[node_id] = 0.0;
    sgdh.r_angle_vec[node_id] = 0.0;
    sgdh.depth_vec[node_id] = 1;
    sgdh.r_depth_vec[node_id] = 1;
    sgdh.flag_vec[node_id] = DIRTY;
    sgdh.origin_vec[node_id] = TOP_LEFT;
    sgdh.parent_vec[node_id] = node_id;
}

/**
 * Create a copy of the Node with the specified node_id.
 **/
Node::
Node(SceneGraphDataHandler& sgdh, const size_t node_id)
 : sgdh(sgdh), node_id(node_id) {
     ++sgdh.ref_vec[node_id];
}

/**
 * Call erase to reduce ref count.
 **/
Node::
~Node() {
    sgdh.erase(node_id);
}

/**
 *
 **/
Node::
Node(const Node& other) : sgdh(other.sgdh), node_id(other.node_id) {
    ++sgdh.ref_vec[node_id];
}

/**
 *
 **/
Node::
Node(Node&& other) noexcept : sgdh(other.sgdh), node_id(other.node_id) {}

/**
 *
 **/
Node& Node::
operator=(const Node& other) {
    if (other.node_id == node_id && &sgdh == &other.sgdh) {
        return *this;
    }
    sgdh = other.sgdh;
    node_id = other.node_id;
    ++sgdh.ref_vec[node_id];
    return *this;
}

/**
 *
 **/
Node& Node::
operator=(Node&& other) noexcept {
    if (other.node_id == node_id && &sgdh == &other.sgdh) {
        return *this;
    }
    sgdh = other.sgdh;
    node_id = other.node_id;
    return *this;
}

/**
 * Attach a new child node.
 **/
Node Node::
attach_node() {
    Node n(sgdh);
    n.set_origin(get_origin());
    n.set_distance_relative(get_distance_relative());
    n.reparent_to(node_id);
    return n;
}

/**
 * Reparent this node to a new parent.
 **/
void Node::
reparent_to(Node& parent) {
    if (sgdh.parent_vec[node_id] != parent.node_id) {
        sgdh.parent_vec[node_id] = parent.node_id;
        propagate_dirty();
    }
}

/**
 * Reparent this node to a new parent.
 **/
void Node::
reparent_to(const size_t parent) {
    if (sgdh.parent_vec[node_id] != parent) {
        sgdh.parent_vec[node_id] = parent;
        propagate_dirty();
    }
}

/**
 * Traverse the scene starting at the root of this Node or optionally only at
 * this Node, if local is true.
 **/
bool Node::
traverse(const bool local) {
    if (local) {
        return scene_traverse(sgdh, node_id);
    }
    size_t root = node_id;
    while (sgdh.parent_vec[root] != root) {
        root = sgdh.parent_vec[root];
    }
    return scene_traverse(sgdh, root);
}

/**
 * Query the scene starting at this node, optionally depth sorted.
 **/
SmallList<size_t> Node::
query(AABB& aabb, const bool depth_sorted) {
    SmallList<size_t> to_process;
    std::vector<DepthSort> v;

    to_process.push_back(node_id);
    while(to_process.size()) {
        const size_t pid = to_process.pop_back();
        if (sgdh.flag_vec[pid] & HIDDEN) {
            continue;
        }
        Node n(sgdh, pid);
        if (aabb.overlap(n.get_aabb())) {
            v.emplace_back(DepthSort(n.node_id, n.get_relative_depth()));
        }
        for (size_t i = 0; i < sgdh.parent_vec.size(); ++i) {
            if (i == pid) {
                continue;
            }
            if (sgdh.parent_vec[i] == pid) {
                to_process.push_back(i);
            }
        }
    }
    if (depth_sorted) {
        std::sort(v.begin(), v.end());
    }
    for (auto& i : v) {
        to_process.push_back(i.node_id);
    }
    return to_process;
}

/**
 * Hide the Node.
 **/
void Node::
hide() {
    if (sgdh.flag_vec[node_id] & HIDDEN) {
        return;
    }
    sgdh.flag_vec[node_id] = sgdh.flag_vec[node_id] | HIDDEN;
}

/**
 * Show the Node.
 **/
void Node::
show() {
    if (!(sgdh.flag_vec[node_id] & HIDDEN)) {
        return;
    }
    sgdh.flag_vec[node_id] = sgdh.flag_vec[node_id] ^ HIDDEN;
    propagate_dirty();
}

/**
 * Propagate the dirty flag to all attached nodes.
 **/
void Node::propagate_dirty() {
    SmallList<size_t> to_process;
    to_process.push_back(node_id);
    while (to_process.size()) {
        const size_t child_node_id = to_process.pop_back();
        sgdh.flag_vec[child_node_id] = sgdh.flag_vec[child_node_id] | DIRTY;
        for (size_t i = 0; i < sgdh.parent_vec.size(); ++i) {
            if (i == node_id || i == child_node_id) {
                continue;
            }
            if (sgdh.parent_vec[i] == child_node_id) {
                to_process.push_back(i);
            }
        }
    }
}

/**
 * Get the Node ID.
 **/
size_t Node::
get_id() {
    return node_id;
}

/**
 * Get the Node ID of the parent.
 **/
size_t Node::
get_parent_id() {
    return sgdh.parent_vec[node_id];
}

/**
 * Set x and y pos to a value.
 **/
void Node::
set_pos(const double v) {
    set_pos(v, v);
}

/**
 * Set the position as x, y.
 **/
void Node::
set_pos(const double x, const double y) {
    if (sgdh.pos_x[node_id] != x || sgdh.pos_y[node_id] != y) {
        sgdh.pos_x[node_id] = x;
        sgdh.pos_y[node_id] = y;
        propagate_dirty();
    }
}

/**
 * Set the position as Vec2.
 **/
void Node::
set_pos(Vec2& p) {
    set_pos(p[0], p[1]);
}

/**
 * Set the position as x, y, relative to another Node.
 **/
void Node::
set_pos(Node& other, const double x, const double y) {
    const double t_x = other.get_relative_x() + x;
    const double t_y = other.get_relative_y() + y;
    if (get_relative_x() == t_x && get_relative_y() == t_y) {
        return;
    }

    const bool dist_rel = (sgdh.flag_vec[node_id] & DISTANCE_RELATIVE) > 0;
    const double p_x = sgdh.pos_x[node_id];
    const double p_y = sgdh.pos_y[node_id];
    const Scale r_s = get_relative_scale();
    const double o_x = get_relative_x() - (dist_rel ? p_x * r_s.sx : p_x);
    const double o_y = get_relative_y() - (dist_rel ? p_y * r_s.sy : p_y);

    sgdh.pos_x[node_id] = t_x - o_x;
    sgdh.pos_y[node_id] = t_y - o_y;
    if (dist_rel) {
        sgdh.pos_x[node_id] /= r_s.sx;
        sgdh.pos_y[node_id] /= r_s.sy;
    }
    propagate_dirty();
}

/**
 * Set x and y pos to a value, relative to another Node.
 **/
void Node::
set_pos(Node& other, const double v) {
    set_pos(other, v, v);
}

/**
 * Set the position as Vec2, relative to another Node.
 **/
void Node::
set_pos(Node& other, Vec2& p) {
    set_pos(other, p[0], p[1]);
}

/**
 * Set the X component.
 **/
void Node::
set_x(const double v) {
    if (sgdh.pos_x[node_id] != v) {
        sgdh.pos_x[node_id] = v;
        propagate_dirty();
    }
}

/**
 * Set the X component, relative to another Node.
 **/
void Node::
set_x(Node& other, const double v) {
    const double t_x = other.get_relative_x() + v;
    const double r_x = get_relative_x();
    if (r_x == t_x) {
        return;
    }

    const bool dist_rel = (sgdh.flag_vec[node_id] & DISTANCE_RELATIVE) > 0;
    const double p_x = sgdh.pos_x[node_id];
    const double s_x = get_relative_scale().sx;
    const double o_x = r_x - (dist_rel ? p_x * s_x : p_x);

    sgdh.pos_x[node_id] = t_x - o_x;
    if (dist_rel) {
        sgdh.pos_x[node_id] /= s_x;
    }
    propagate_dirty();
}

/**
 * Set the Y component.
 **/
void Node::
set_y(const double v) {
    if (sgdh.pos_y[node_id] != v) {
        sgdh.pos_y[node_id] = v;
        propagate_dirty();
    }
}

/**
 * Set the Y component, relative to another Node.
 **/
void Node::
set_y(Node& other, const double v) {
    const double t_y = other.get_relative_y() + v;
    const double r_y = get_relative_y();
    if (r_y == t_y) {
        return;
    }

    const bool dist_rel = (sgdh.flag_vec[node_id] & DISTANCE_RELATIVE) > 0;
    const double p_y = sgdh.pos_y[node_id];
    const double s_y = get_relative_scale().sy;
    const double o_y = r_y - (dist_rel ? p_y * s_y : p_y);

    sgdh.pos_y[node_id] = t_y - o_y;
    if (dist_rel) {
        sgdh.pos_y[node_id] /= s_y;
    }
    propagate_dirty();
}

/**
 * Get the position as Vec2.
 **/
Vec2 Node::
get_pos() {
    return Vec2(sgdh.pos_x[node_id], sgdh.pos_y[node_id]);
}

/**
 * Get the position as Vec2, relative to another Node.
 **/
Vec2 Node::
get_pos(Node& other) {
    return get_relative_pos() - other.get_relative_pos();
}

/**
 * Get the relative position as Vec2.
 **/
Vec2 Node::
get_relative_pos() {
    clean_node();
    return Vec2(sgdh.r_pos_x[node_id], sgdh.r_pos_y[node_id]);
}

/**
 * Get X component of the position.
 **/
double Node::
get_x() {
    return sgdh.pos_x[node_id];
}

/**
 * Get X component of the position, relative to another Node.
 **/
double Node::
get_x(Node& other) {
    return get_relative_x() - other.get_relative_x();
}

/**
 * Get X component of the relative position.
 **/
double Node::
get_relative_x() {
    clean_node();
    return sgdh.r_pos_x[node_id];
}

/**
 * Get Y component of the position.
 **/
double Node::
get_y() {
    return sgdh.pos_y[node_id];
}

/**
 * Get Y component of the position, relative to another Node.
 **/
double Node::
get_y(Node& other) {
    return get_relative_y() - other.get_relative_y();
}

/**
 * Get Y component of the relative position.
 **/
double Node::
get_relative_y() {
    clean_node();
    return sgdh.r_pos_y[node_id];
}

/**
 * Set the scale as single value double.
 **/
void Node::
set_scale(const double s) {
    set_scale(s, s);
}

/**
 * Set the scale as two value double.
 **/
void Node::
set_scale(const double sx, const double sy) {
    if (sgdh.scale_x[node_id] != sx || sgdh.scale_y[node_id] != sy) {
        sgdh.scale_x[node_id] = sx;
        sgdh.scale_y[node_id] = sy;
        propagate_dirty();
    }
}

/**
 * Set the scale as Scale.
 **/
void Node::
set_scale(const Scale& s) {
    set_scale(s.sx, s.sy);
}

/**
 * Set the scale as single value double, relative to another Node.
 **/
void Node::
set_scale(Node& other, const double s) {
    set_scale(other, s, s);
}

/**
 * Set the scale as two value double, relative to another Node.
 **/
void Node::
set_scale(Node& other, const double sx, const double sy) {
    Scale t_scale = other.get_relative_scale();
    t_scale.sx = t_scale.sx * sx;
    t_scale.sy = t_scale.sy * sy;

    Scale r_scale = get_relative_scale();

    if (r_scale != t_scale) {
        sgdh.scale_x[node_id] = sgdh.scale_x[node_id] / r_scale.sx * t_scale.sx;
        sgdh.scale_y[node_id] = sgdh.scale_y[node_id] / r_scale.sy * t_scale.sy;
        propagate_dirty();
    }
}

/**
 * Set the scale as Scale.
 **/
void Node::
set_scale(Node& other, const Scale& s) {
    set_scale(other, s.sx, s.sy);
}

/**
 * Get scale as Scale.
 **/
Scale Node::
get_scale() {
    return Scale(sgdh.scale_x[node_id], sgdh.scale_y[node_id]);
}

/**
 * Get scale as Scale, relative to another Node.
 **/
Scale Node::
get_scale(Node& other) {
    Scale o_s = other.get_relative_scale();
    Scale t_s = get_relative_scale();
    return Scale(t_s.sx / o_s.sx, t_s.sy / o_s.sy);
}

/**
 * Get relative scale as Scale.
 **/
Scale Node::
get_relative_scale() {
    clean_node();
    return Scale(sgdh.r_scale_x[node_id], sgdh.r_scale_y[node_id]);
}

/**
 * Set angle to double in degrees or radians if true.
 **/
void Node::
set_angle(const double angle, bool radians) {
    const double deg_angle = radians ? angle * to_deg : angle;
    if (sgdh.angle_vec[node_id] != deg_angle) {
        sgdh.angle_vec[node_id] = deg_angle;
        propagate_dirty();
    }
}

/**
 * Set angle to double in degrees or radians if true, relative to another Node.
 **/
void Node::
set_angle(Node& other, const double angle, bool radians) {
    const double deg_angle = radians ? angle * to_deg : angle;
    const double t_angle = other.get_relative_angle() + deg_angle;
    const double r_angle = get_relative_angle();
    if (r_angle != t_angle) {
        sgdh.angle_vec[node_id] += r_angle - t_angle;
        propagate_dirty();
    }
}

/**
 * Get the angle in degrees or radians if true.
 **/
double Node::
get_angle(bool radians) {
    return sgdh.angle_vec[node_id] * (radians ? to_rad : 1.0);
}

/**
 * Get the angle in degrees or radians if true, relative to another Node.
 **/
double Node::
get_angle(Node& other, bool radians) {
    const double a = other.get_relative_angle() - get_relative_angle();
    return a * (radians ? to_rad : 1.0);
}

/**
 * Get the relative angle in degrees.
 **/
double Node::
get_relative_angle() {
    clean_node();
    return sgdh.r_angle_vec[node_id];
}

/**
 * Set the rotation center to two value double.
 **/
void Node::
set_rotation_center(const double x, const double y) {
    if (!(sgdh.flag_vec[node_id] & ROTATION_CENTER_SET) ||
            sgdh.rotation_center_x[node_id] != x ||
            sgdh.rotation_center_y[node_id] != y) {
        sgdh.rotation_center_x[node_id] = x;
        sgdh.rotation_center_y[node_id] = y;
        sgdh.flag_vec[node_id] = sgdh.flag_vec[node_id] | ROTATION_CENTER_SET;
        propagate_dirty();
    }
}

/**
 * Set the rotation center to Vec2.
 **/
void Node::
set_rotation_center(Vec2& c) {
    if (!(sgdh.flag_vec[node_id] & ROTATION_CENTER_SET) ||
            sgdh.rotation_center_x[node_id] != c[0] ||
            sgdh.rotation_center_y[node_id] != c[1]) {
        sgdh.rotation_center_x[node_id] = c[0];
        sgdh.rotation_center_y[node_id] = c[1];
        sgdh.flag_vec[node_id] = sgdh.flag_vec[node_id] | ROTATION_CENTER_SET;
        propagate_dirty();
    }
}

/**
 * Reset rotation center.
 **/
void Node::
reset_rotation_center() {
    if (sgdh.flag_vec[node_id] & ROTATION_CENTER_SET) {
        sgdh.flag_vec[node_id] = sgdh.flag_vec[node_id] ^ ROTATION_CENTER_SET;
    }
    propagate_dirty();
}

/**
 * Get rotation center. If not set, returns the center.
 **/
Vec2 Node::
get_rotation_center() {
    if (!(sgdh.flag_vec[node_id] & ROTATION_CENTER_SET)) {
        return Vec2(
            sgdh.size_x[node_id] / 2.0,
            sgdh.size_y[node_id] / 2.0
        );
    }
    return Vec2(
        sgdh.rotation_center_x[node_id],
        sgdh.rotation_center_y[node_id]
    );
}

/**
 * Set depth.
 **/
void Node::
set_depth(const int depth) {
    if (sgdh.depth_vec[node_id] != depth) {
        sgdh.depth_vec[node_id] = depth;
        propagate_dirty();
    }
}

/**
 * Set depth, relative to another Node.
 **/
void Node::
set_depth(Node& other, const int depth) {
    const int d = other.get_relative_depth() + depth - get_relative_depth();
    if (d != 0) {
        sgdh.depth_vec[node_id] += d;
        propagate_dirty();
    }
}

/**
 * Get depth.
 **/
int Node::
get_depth() {
    return sgdh.depth_vec[node_id];
}

/**
 * Get depth, relative to another Node.
 **/
int Node::
get_depth(Node& other) {
    return other.get_relative_depth() - get_relative_depth();
}

/**
 * Get relative depth, relative to another Node. !!!!
 **/
int Node::
get_relative_depth() {
    clean_node();
    return sgdh.r_depth_vec[node_id];
}

/**
 * Set size as two value double.
 **/
void Node::
set_size(const double x, const double y) {
    if (sgdh.size_x[node_id] != x || sgdh.size_y[node_id] != y) {
        sgdh.size_x[node_id] = x;
        sgdh.size_y[node_id] = y;
        propagate_dirty();
    }
}

/**
 * Set size as Size.
 **/
void Node::
set_size(const Size& s) {
    if (sgdh.size_x[node_id] != s.w || sgdh.size_y[node_id] != s.h) {
        sgdh.size_x[node_id] = s.w;
        sgdh.size_y[node_id] = s.h;
        propagate_dirty();
    }
}

/**
 * Get size as Size.
 **/
Size Node::
get_size() {
    return Size(sgdh.size_x[node_id], sgdh.size_y[node_id]);
}

/**
 * Get relative size as Size.
 **/
Size Node::
get_relative_size() {
    clean_node();
    return Size(
        sgdh.size_x[node_id] * sgdh.r_scale_x[node_id],
        sgdh.size_y[node_id] * sgdh.r_scale_y[node_id]
    );
}

/**
 * Set origin.
 **/
void Node::
set_origin(Origin o) {
    if (sgdh.origin_vec[node_id] != o) {
        sgdh.origin_vec[node_id] = o;
        propagate_dirty();
    }
}

/**
 * Get origin.
 **/
Origin Node::
get_origin() {
    return sgdh.origin_vec[node_id];
}

/**
 * Set or remove relative distance flag.
 **/
void Node::
set_distance_relative(const bool v) {
    if (!(sgdh.flag_vec[node_id] & DISTANCE_RELATIVE) && v) {
        sgdh.flag_vec[node_id] = sgdh.flag_vec[node_id] | DISTANCE_RELATIVE;
        propagate_dirty();
    }
    else if ((sgdh.flag_vec[node_id] & DISTANCE_RELATIVE) && !v) {
        sgdh.flag_vec[node_id] = sgdh.flag_vec[node_id] ^ DISTANCE_RELATIVE;
        propagate_dirty();
    }
}

/**
 * Get whether the relative distance flag has been set.
 **/
bool Node::
get_distance_relative() {
    return (sgdh.flag_vec[node_id] & DISTANCE_RELATIVE) > 0;
}

/**
 * Return the AABB of the Node.
 **/
AABB Node::
get_aabb() {
    Size r_sz = get_relative_size();
    Vec2 r_pos = get_relative_pos();
    Scale r_sc = get_relative_scale();

    double min_x = r_pos[0], max_x = r_pos[0] + r_sz.w;
    double min_y = r_pos[1], max_y = r_pos[1] + r_sz.h;
    Vec2 top_left(min_x, min_y);
    Vec2 top_right(max_x, min_y);
    Vec2 bottom_left(min_x, max_y);
    Vec2 bottom_right(max_x, max_y);
    Vec2 rot_center = get_rotation_center();
    rot_center = Vec2(rot_center[0] * r_sc.sx, rot_center[1] * r_sc.sy) + r_pos;
    Vec2 rot;
    const double a = get_relative_angle();

    rot = (top_left - rot_center).rotated(a) + rot_center;
    min_x = std::min(min_x, rot[0]);
    max_x = std::max(max_x, rot[0]);
    min_y = std::min(min_y, rot[1]);
    max_y = std::max(max_y, rot[1]);

    rot = (top_right - rot_center).rotated(a) + rot_center;
    min_x = std::min(min_x, rot[0]);
    max_x = std::max(max_x, rot[0]);
    min_y = std::min(min_y, rot[1]);
    max_y = std::max(max_y, rot[1]);

    rot = (bottom_left - rot_center).rotated(a) + rot_center;
    min_x = std::min(min_x, rot[0]);
    max_x = std::max(max_x, rot[0]);
    min_y = std::min(min_y, rot[1]);
    max_y = std::max(max_y, rot[1]);

    rot = (bottom_right - rot_center).rotated(a) + rot_center;
    min_x = std::min(min_x, rot[0]);
    max_x = std::max(max_x, rot[0]);
    min_y = std::min(min_y, rot[1]);
    max_y = std::max(max_y, rot[1]);

    const double hw = (max_x - min_x) / 2.0;
    const double hh = (max_y - min_y) / 2.0;
    return AABB(min_x + hw, min_y + hh, hw, hh);
}


// Systems

/**
 * Traverse the scene graph, optionally starting at a specified Node. Traversal
 * first finds the first either non dirty or root node (depending on which is
 * found first) and traverses the scene from that node.
 **/
bool scene_traverse(SceneGraphDataHandler& sgdh, const size_t start_node) {
    bool was_dirty = false;
    const size_t parent = sgdh.parent_vec[start_node];
    if (parent != start_node && (sgdh.flag_vec[start_node] & DIRTY)) {
        minimal_clean(sgdh, parent);
        was_dirty = true;
    }

    SmallList<size_t> to_process;
    SmallList<size_t> nodes;
    nodes.reserve(sgdh.flag_vec.size());

    nodes.push_back(start_node);
    to_process.push_back(start_node);
    size_t current_proc_id = 0;
    while (to_process.size() > current_proc_id) {
        const size_t pid = to_process[current_proc_id];
        for (size_t i = 0; i < sgdh.parent_vec.size(); ++i) {
            if (i == pid || sgdh.flag_vec[i] & FREE) {
                continue;
            }
            if (sgdh.parent_vec[i] == pid) {
                to_process.push_back(i);
                nodes.push_back(i);
            }
        }
        ++current_proc_id;
    }

    process_angle(sgdh, nodes);
    process_depth(sgdh, nodes);
    process_scale(sgdh, nodes);
    process_pos(sgdh, nodes);
    if (clear_dirty_flag(sgdh, nodes) && !was_dirty) {
        was_dirty = true;
    }

    return was_dirty;
}


/**
 * Perform the least amount of traversal to clean a Node.
 **/
void minimal_clean(SceneGraphDataHandler &sgdh, const size_t node_id) {
    SmallList<size_t> path;
    path.push_back(node_id);
    dirty_path(sgdh, node_id, path);
    path.reverse();
    process_angle(sgdh, path);
    process_depth(sgdh, path);
    process_scale(sgdh, path);
    process_pos(sgdh, path);
    clear_dirty_flag(sgdh, path);
}

/**
 * Populates a list of dirty node ids to traverse, to get to the specified
 * Node in reversed order. Does not clear the list.
 **/
void dirty_path(SceneGraphDataHandler &sgdh, const size_t node_id,
                SmallList<size_t>& path) {
    size_t base_node = node_id;
    while (base_node > 0 && sgdh.parent_vec[base_node] != base_node) {
        if (!(sgdh.flag_vec[base_node] & DIRTY)) {
            break;
        }
        if (sgdh.flag_vec[base_node] & FREE) {
            throw std::runtime_error("Encountered a removed Node.");
        }
        base_node = sgdh.parent_vec[base_node];
        path.push_back(base_node);
    }
}

/**
 * Process all angles of nodes in path.
 **/
void process_angle(SceneGraphDataHandler& sgdh, SmallList<size_t>& path) {
    for (size_t i = 0; i < path.size(); ++i) {
        const size_t pid = path[i];
        const size_t parent = sgdh.parent_vec[pid];
        const double rel = parent == pid ? 0.0 : sgdh.r_angle_vec[parent];
        sgdh.r_angle_vec[pid] = rel + sgdh.angle_vec[pid];
    }
}

/**
 * Process all depths of nodes in path.
 **/
void process_depth(SceneGraphDataHandler& sgdh, SmallList<size_t>& path) {
    for (size_t i = 0; i < path.size(); ++i) {
        const size_t pid = path[i];
        const size_t parent = sgdh.parent_vec[pid];
        const double rel = parent == pid ? 0 : sgdh.r_depth_vec[parent];
        sgdh.r_depth_vec[pid] = rel + sgdh.depth_vec[pid];
    }
}

/**
 * Process all scale factors of nodes in path.
 **/
void process_scale(SceneGraphDataHandler& sgdh, SmallList<size_t>& path) {
    for (size_t i = 0; i < path.size(); ++i) {
        const size_t pid = path[i];
        const size_t parent = sgdh.parent_vec[pid];
        const double rel = parent == pid ? 1.0 : sgdh.r_scale_x[parent];
        sgdh.r_scale_x[pid] = rel * sgdh.scale_x[pid];
    }
    for (size_t i = 0; i < path.size(); ++i) {
        const size_t pid = path[i];
        const size_t parent = sgdh.parent_vec[pid];
        const double rel = parent == pid ? 1.0 : sgdh.r_scale_y[parent];
        sgdh.r_scale_y[pid] = rel * sgdh.scale_y[pid];
    }
}

/**
 * Process all positions of nodes in path.
 **/
void process_pos(SceneGraphDataHandler& sgdh, SmallList<size_t>& path) {
    for (size_t i = 0; i < path.size(); ++i) {
        const size_t pid = path[i];
        const size_t parent = sgdh.parent_vec[pid];
        const double rel_x = parent == pid ? 0.0 : sgdh.r_pos_x[parent];
        const double rel_y = parent == pid ? 0.0 : sgdh.r_pos_y[parent];
        const double hw = sgdh.size_x[pid] / 2.0, hh = sgdh.size_y[pid] / 2.0;
        const bool dist_rel = (sgdh.flag_vec[pid] & DISTANCE_RELATIVE) > 0;
        const double sx = sgdh.r_scale_x[pid];
        const double sy = sgdh.r_scale_y[pid];

        double x = sgdh.pos_x[pid], y = sgdh.pos_y[pid];
        x -= (sgdh.origin_vec[pid] % 3) * hw;
        y -= (sgdh.origin_vec[pid] / 3) * hh;
        if (dist_rel) {
            x = dist_rel ? x * sx : x;
            y = dist_rel ? y * sy : y;
        }
        if (sgdh.r_angle_vec[pid] != 0.0) {
            const double rad = sgdh.r_angle_vec[pid] * -to_rad;
            const double sa = std::sin(rad), ca = std::cos(rad);
            double rot_cen_x, rot_cen_y;
            if (sgdh.flag_vec[pid] & ROTATION_CENTER_SET) {
                rot_cen_x = x + sgdh.rotation_center_x[pid] * sx;
                rot_cen_y = y + sgdh.rotation_center_y[pid] * sy;
            }
            else {
                rot_cen_x = x + hw * sx;
                rot_cen_y = y + hh * sy;
            }
            const double tmp_x = ca * (x - rot_cen_x) - sa * (y - rot_cen_y);
            const double tmp_y = sa * (x - rot_cen_x) + ca * (y - rot_cen_y);
            x = tmp_x + rot_cen_x;
            y = tmp_y + rot_cen_y;
        }
        sgdh.r_pos_x[pid] = rel_x + x;
        sgdh.r_pos_y[pid] = rel_y + y;
    }
}

/**
 * Clear all dirty flags.
 **/
bool clear_dirty_flag(SceneGraphDataHandler& sgdh, SmallList<size_t>& path) {
    bool dirty = false;
    int count = 0;
    for (size_t i = 0; i < path.size(); ++i) {
        if (sgdh.flag_vec[path[i]] & DIRTY) {
            if (!dirty) {
                dirty = true;
            }
            sgdh.flag_vec[path[i]] = sgdh.flag_vec[path[i]] ^ DIRTY;
            ++count;
        }
    }
    return dirty;
}


}  // namespace scene
}  // namespace foolysh
