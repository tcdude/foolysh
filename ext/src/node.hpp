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
 * Node implementation.
 */

#ifndef NODE_HPP
#define NODE_HPP

#include <vector>

#include "vec2.hpp"
#include "list_t.hpp"
#include "aabb.hpp"

namespace foolysh {
namespace scene {

typedef foolysh::tools::Vec2 Vec2;
typedef foolysh::tools::AABB AABB;
using tools::SmallList;

enum Flags {
    DIRTY = 1,
    ROTATION_CENTER_SET = 2,
    DISTANCE_RELATIVE = 4,
    HIDDEN = 8,
    FREE = 16
};

enum Origin {
    TOP_LEFT = 0,
    TOP_CENTER = 1,
    TOP_RIGHT = 2,
    CENTER_LEFT = 3,
    CENTER = 4,
    CENTER_RIGHT = 5,
    BOTTOM_LEFT = 6,
    BOTTOM_CENTER = 7,
    BOTTOM_RIGHT = 8
};

struct Scale {
    Scale() : sx(1.0), sy(1.0) {}
    Scale(const double _s) : sx(_s), sy(_s) {}
    Scale(const double _sx, const double _sy) : sx(_sx), sy(_sy) {}

    double sx, sy;
    Scale operator+(const Scale& rhs) {
        Scale s = {sx + rhs.sx, sy + rhs.sy};
        return s;
    }
    Scale operator-(const Scale& rhs) {
        Scale s = {sx - rhs.sx, sy - rhs.sy};
        return s;
    }
    Scale operator*(const Scale& rhs) {
        Scale s = {sx * rhs.sx, sy * rhs.sy};
        return s;
    }
    Scale operator*(const double rhs) {
        Scale s = {sx * rhs, sy * rhs};
        return s;
    }
    bool operator==(const double rhs) {
        return (sx == rhs && sy == rhs);
    }
    bool operator!=(const double rhs) {
        return (sx != rhs || sy != rhs);
    }
    bool operator==(const Scale& rhs) {
        return (sx == rhs.sx && sy == rhs.sy);
    }
    bool operator!=(const Scale& rhs) {
        return (sx != rhs.sx || sy != rhs.sy);
    }
};

struct Size {
    Size() : w(0.0), h(0.0) {}
    Size(const double w, const double h) : w(w), h(h) {}
    double w = 0.0, h = 0.0;
    Size operator*(const Scale& rhs) {
        Size s;
        s.w = w * rhs.sx;
        s.h = h * rhs.sy;
        return s;
    }
};

struct DepthSort {
    DepthSort(const size_t n, const int d) : node_id(n), depth(d) {}
    size_t node_id;
    int depth;
    bool operator<(DepthSort& other) {
        return depth < other.depth;
    }
};

/**
 * Holds all data vectors for one scene graph with index 0 being the root.
 * Prefix meanings:
 *   r_ = relative = top left point (as used/needed in SDL).
 *   o_ = origin = local origin of the Node with rotation and scale applied.
 *
 * Flags: 1 = dirty, 2 = rotation_center set, 4 = scaled_position, 8 = hidden,
 *        16 = free
 **/
struct SceneGraphDataHandler {
    SceneGraphDataHandler() {}
    std::vector<double> pos_x;
    std::vector<double> pos_y;
    std::vector<double> r_pos_x;
    std::vector<double> r_pos_y;
    std::vector<double> o_pos_x;
    std::vector<double> o_pos_y;
    std::vector<double> scale_x;
    std::vector<double> scale_y;
    std::vector<double> r_scale_x;
    std::vector<double> r_scale_y;
    std::vector<double> size_x;
    std::vector<double> size_y;
    std::vector<double> rotation_center_x;
    std::vector<double> rotation_center_y;
    std::vector<double> angle_vec;
    std::vector<double> r_angle_vec;
    std::vector<int> depth_vec;
    std::vector<int> r_depth_vec;
    std::vector<unsigned char> flag_vec;
    std::vector<Origin> origin_vec;
    std::vector<size_t> parent_vec;
    std::vector<size_t> free_vec;
    std::vector<size_t> ref_vec;

    size_t get_empty();
    void erase(const size_t node_id);
    void reserve(const size_t size);
};


// Systems

bool scene_traverse(SceneGraphDataHandler& sgdh, const size_t start_node = 0);
void minimal_clean(SceneGraphDataHandler& sgdh, const size_t node_id);
void dirty_path(SceneGraphDataHandler& sgdh, const size_t node_id,
                SmallList<size_t>& path);
void process_angle(SceneGraphDataHandler& sgdh, SmallList<size_t>& path);
void process_depth(SceneGraphDataHandler& sgdh, SmallList<size_t>& path);
void process_scale(SceneGraphDataHandler& sgdh, SmallList<size_t>& path);
void process_origin(SceneGraphDataHandler& sgdh, SmallList<size_t>& path);
void process_pos(SceneGraphDataHandler& sgdh, SmallList<size_t>& path);
bool clear_dirty_flag(SceneGraphDataHandler& sgdh, SmallList<size_t>& path);


// Node

class Node {
public:
    Node(SceneGraphDataHandler& sgdh);
    Node(SceneGraphDataHandler& sgdh, const size_t node_id);
    ~Node();
    Node(const Node& other);
    Node(Node&& other) noexcept;
    Node& operator=(const Node& other);
    Node& operator=(Node&& other) noexcept;

    Node attach_node();
    void reparent_to(Node& parent);
    void reparent_to(const size_t parent);
    bool traverse(const bool local = false);
    SmallList<size_t> query(AABB& aabb, const bool depth_sorted = true);
    bool hidden();
    void hide();
    void show();
    void propagate_dirty();

    size_t get_id();
    size_t get_parent_id();

    void set_pos(const double x, const double y);
    void set_pos(Node& other, const double x, const double y);
    void set_pos(const double v);
    void set_pos(Node& other, const double v);
    void set_pos(Vec2& p);
    void set_pos(Node& other, Vec2& p);
    void set_x(const double v);
    void set_x(Node& other, const double v);
    void set_y(const double v);
    void set_y(Node& other, const double v);
    Vec2 get_pos();
    Vec2 get_pos(Node& other);
    Vec2 get_relative_pos();
    double get_x();
    double get_x(Node& other);
    double get_relative_x();
    double get_y();
    double get_y(Node& other);
    double get_relative_y();

    void set_scale(const double s);
    void set_scale(Node& other, const double s);
    void set_scale(const double sx, const double sy);
    void set_scale(Node& other, const double sx, const double sy);
    void set_scale(const Scale& s);
    void set_scale(Node& other, const Scale& s);
    Scale get_scale();
    Scale get_scale(Node& other);
    Scale get_relative_scale();

    void set_angle(double a, bool radians = false);
    void set_angle(Node& other, double a, bool radians = false);
    double get_angle(bool radians = false);
    double get_angle(Node& other, bool radians = false);
    double get_relative_angle();

    void set_rotation_center(const double x, const double y);
    void set_rotation_center(Vec2& c);
    void reset_rotation_center();
    Vec2 get_rotation_center();

    void set_depth(const int d);
    void set_depth(Node& other, const int d);
    int get_depth();
    int get_depth(Node& other);
    int get_relative_depth();

    void set_size(const Size& s);
    void set_size(const double w, const double h);
    Size get_size();
    Size get_relative_size();

    void set_origin(Origin o);
    Origin get_origin();

    void set_distance_relative(const bool v);  // Whether to scale distance
    bool get_distance_relative();

    AABB get_aabb();

private:
    SceneGraphDataHandler& sgdh;
    size_t node_id;

    inline void clean_node() {
        if (sgdh.flag_vec[node_id] & DIRTY)  {
            minimal_clean(sgdh, node_id);
        }
    }
};

}  // namespace scene
}  // namespace foolysh

#endif
