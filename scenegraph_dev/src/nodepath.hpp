/**
 * NodePath implementation
 */

#ifndef NODEPATH_HPP
#define NODEPATH_HPP

#include "vector2.hpp"
#include "quadtree.hpp"
#include "list_t.hpp"
#include "aabb.hpp"

#include <memory>
#include <vector>


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

    struct Size {
        double w = 0.0, h = 0.0;
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

    struct ChildNodePath {
        int node_path_id;
        int next = -1;
    };

    struct QuadtreeEntry {
        int node_path_id;
        AABB aabb;
    };

    class NodePath {
    public:
        NodePath();
        ~NodePath();
        NodePath(const NodePath& other);
        NodePath(NodePath&& other) noexcept;
        NodePath& operator=(const NodePath& other);
        NodePath& operator=(NodePath&& other) noexcept;

        NodePath attach_new_node_path();
        void reparent_to(NodePath& parent);
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

        void set_size(const Size& s);
        void set_size(const double w, const double h);
        Size get_size();
        void set_distance_relative(const bool v);
        bool get_distance_relative();

        static void get_node_path(const int node_path_id, NodePath& np);

    private:
        void _update_relative();
        void _update_aabb();
        Vector2 _get_offset();
        Vector2 _get_rotation_center_offset();
        void _insert_child(const int node_path_id);
        void _remove_child(const int node_path_id);
        void _propagate_dirty();

        int _node_id;
        std::unique_ptr<Vector2> _position, _r_position, _rot_center;
        std::unique_ptr<std::vector<QuadtreeEntry>> _tmp_quadtree_entry;
        double _angle, _r_angle;
        Scale _scale, _r_scale;
        Size _size;
        int _depth, _r_depth;
        bool _dirty, _distance_relative, _hidden, _isnew;
        std::unique_ptr<Quadtree> _quadtree;
        int _parent, _first_child;
        Origin _origin;
        AABB _aabb;

        static ExtFreeList<NodePath*> _node_paths;
        static ExtFreeList<ChildNodePath> _child_node_paths;
        static int _max_qt_leaf_elements, _max_qt_depth;
    };

    ExtFreeList<NodePath*> NodePath::_node_paths;
    ExtFreeList<ChildNodePath> NodePath::_child_node_paths;
    int NodePath::_max_qt_leaf_elements = 8;
    int NodePath::_max_qt_depth = 8;
}  // namespace scenegraph

#endif 