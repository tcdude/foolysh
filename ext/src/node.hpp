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
 * Node implementation.
 */

#ifndef NODE_HPP
#define NODE_HPP

#include "vector2.hpp"
#include "quadtree.hpp"
#include "list_t.hpp"
#include "aabb.hpp"

#include <memory>
#include <vector>

using tools::AABB;
using tools::Vector2;
using tools::Quadtree;
using tools::FreeList;
using tools::ExtFreeList;
using tools::SmallList;

namespace scenegraph {
    enum Origin {
        TOP_LEFT,
        TOP_RIGHT,
        TOP_CENTER,
        CENTER_LEFT,
        CENTER,
        CENTER_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT,
        BOTTOM_CENTER,
    };

    struct Scale {
        double sx = 1.0, sy = 1.0;
        Scale operator*(const Scale& rhs) {
            Scale s;
            s.sx = sx * rhs.sx;
            s.sy = sy * rhs.sy;
            return s;
        }
        bool operator==(const double rhs) {
            return (sx == rhs && sy == rhs);
        }
        bool operator!=(const double rhs) {
            return (sx != rhs || sy != rhs);
        }
    };

    struct Size {
        double w = 0.0, h = 0.0;
        Size operator*(const Scale& rhs) {
            Size s;
            s.w = w * rhs.sx;
            s.h = h * rhs.sy;
            return s;
        }
    };

    struct ChildNode {
        int node_id;
        int next = -1;
    };

    struct QuadtreeEntry {
        int node_id;
        AABB aabb;
    };

    struct NodeData {
        NodeData();
        ~NodeData();
        NodeData& operator=(const NodeData& other);

        bool _traverse();
        SmallList<int> _query(AABB& aabb);
        void _insert_child(const int node_id);
        void _remove_child(const int node_id);
        void _propagate_dirty();
        void _update_relative();
        Vector2 _get_offset();

        Vector2 _position, _r_position, _tl_position, _rot_center;
        std::vector<QuadtreeEntry> _tmp_quadtree_entry;
        double _angle, _r_angle;
        Scale _scale, _r_scale;
        Size _size, _r_size;
        int _depth, _r_depth;
        bool _dirty, _distance_relative, _hidden, _isnew;
        std::unique_ptr<Quadtree> _quadtree;
        int _parent, _first_child, _node_data_id;
        Origin _origin;
        AABB _aabb;
        unsigned int _ref_count;

        static ExtFreeList<ChildNode> _child_nodes;
        static ExtFreeList<NodeData*> _nd;
        static int _max_qt_leaf_elements, _max_qt_depth;
    };
    ExtFreeList<NodeData*> NodeData::_nd;
    ExtFreeList<ChildNode> NodeData::_child_nodes;
    int NodeData::_max_qt_leaf_elements = 8;
    int NodeData::_max_qt_depth = 8;

    class Node {
    public:
        Node();
        ~Node();
        Node(const Node& other);
        Node(Node&& other) noexcept;
        Node& operator=(const Node& other);
        Node& operator=(Node&& other) noexcept;

        Node attach_node();
        void reparent_to(Node& parent);
        void reparent_to(const int parent);
        bool traverse();
        SmallList<int> query(AABB& aabb);
        void hide();
        void show();

        int get_id();
        void set_pos(const double v);
        void set_pos(const double x, const double y);
        void set_pos(Vector2& p);
        Vector2 get_pos();
        void set_scale(const double s);
        void set_scale(const double sx, const double sy);
        void set_scale(const Scale& s);
        Scale get_scale();
        void set_angle(double a, bool radians = false);
        double get_angle(bool radians = false);
        void set_rotation_center(const double x, const double y);
        void set_rotation_center(Vector2& c);
        Vector2 get_rotation_center();
        void set_depth(const int d);
        int get_depth();
        void set_origin(Origin o);
        Origin get_origin();

        Vector2 get_relative_pos();
        Scale get_relative_scale();
        double get_relative_angle();
        int get_relative_depth();
        Size get_relative_size();

        void set_size(const Size& s);
        void set_size(const double w, const double h);
        Size get_size();
        void set_distance_relative(const bool v);
        bool get_distance_relative();

        AABB get_aabb();

    private:
        inline NodeData& _get_root();
        inline NodeData& _get_node_data(const int node_id);

        int _node_id;

    };
}  // namespace scenegraph

#endif