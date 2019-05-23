/**
 * NodePath implementation
 */

#include "nodepath.hpp"

#include <algorithm>
#include <stdexcept>
#include <limits>

constexpr double to_deg = 180.0 / 3.14159265358979323846;
constexpr double to_rad = 3.14159265358979323846 / 180.0;

/**
 * Default Constructor.
 */
scenegraph::NodePath::
NodePath() {
    _node_id = _node_paths.insert(this);
    _position.reset(new Vector2(0.0));
    _r_position.reset(new Vector2(0.0));
    _rot_center.reset(new Vector2(0.0));
    _dirty = _isnew = true;
    _distance_relative = _hidden = false;
    _parent = -1;
    _first_child = -1;
    _angle = _r_angle = 0.0;
    _depth = _r_depth = 1;
    _origin = TOP_LEFT;
}

/**
 * Destructor, clean up relations.
 */
scenegraph::NodePath::
~NodePath() {
    _node_paths.erase(_node_id);
    if (_parent > -1) {
        if (NodePath::_node_paths.active(_parent)) {
            NodePath& np = *NodePath::_node_paths[_parent];
            np._remove_child(_node_id);
        }
    }
}

/**
 * Copy Constructor.
 */
scenegraph::NodePath::
NodePath(const NodePath& other) {
    _node_id = _node_paths.insert(this);
    _position.reset(new Vector2(*other._position));
    _r_position.reset(new Vector2(*other._r_position));
    _rot_center.reset(new Vector2(*other._r_position));
    _dirty = _isnew = true;
    _hidden = other._hidden;
    _distance_relative = other._distance_relative;
    _parent = other._parent;
    if (_parent > -1) {
        if (!NodePath::_node_paths.active(_parent)) {
            throw std::runtime_error("Parent NodePath does not exist!");
        }
        NodePath& np = *NodePath::_node_paths[_parent];
        np._insert_child(_node_id);
    }
    _first_child = -1;
    _angle = other._angle;
    _r_angle = other._r_angle;
    _depth = other._depth;
    _r_depth = other._r_depth;
    _origin = other._origin;
}

/**
 * Move Constructor.
 */
scenegraph::NodePath::
NodePath(NodePath&& other) noexcept {
    _node_id = _node_paths.insert(this);
    _position.reset(new Vector2(*other._position));
    _r_position.reset(new Vector2(*other._r_position));
    _rot_center.reset(new Vector2(*other._r_position));
    _dirty = _isnew = true;
    _hidden = other._hidden;
    _distance_relative = other._distance_relative;
    _parent = other._parent;
    if (_parent > -1) {
        if (!NodePath::_node_paths.active(_parent)) {
            throw std::runtime_error("Parent NodePath does not exist!");
        }
        NodePath& np = *NodePath::_node_paths[_parent];
        np._insert_child(_node_id);
    }
    _first_child = other._first_child;
    if (_first_child > -1) {
        SmallList<int> to_process;
        to_process.push_back(_first_child);
        while (to_process.size() > 0) {
            const int cnp_id = to_process.pop_back();
            ChildNodePath& cnp = NodePath::_child_node_paths[cnp_id];
            NodePath& np = *NodePath::_node_paths[cnp.node_path_id];
            np._parent = _node_id;
            if (cnp.next != -1) {
                to_process.push_back(cnp.next);
            }
        }
    }
    _angle = other._angle;
    _r_angle = other._r_angle;
    _depth = other._depth;
    _r_depth = other._r_depth;
    _origin = other._origin;
}

/**
 * Copy Assignment Operator.
 */
scenegraph::NodePath& scenegraph::NodePath::
operator=(const NodePath& other) {
    if (&other != this) {
        *this = NodePath(other);
    }
    return *this;
}

/**
 * Move Assignment Operator.
 */
scenegraph::NodePath& scenegraph::NodePath::
operator=(NodePath&& other) noexcept {
    if (&other != this) {
        *this = NodePath(other);
    }
    return *this;
}

/**
 * Attach new child NodePath to this.
 */
scenegraph::NodePath scenegraph::NodePath::
attach_new_node_path() {
    NodePath np;
    np._parent = _node_id;
    _insert_child(np._node_id);
    _propagate_dirty();
    return np;
}

/**
 * Reparents this NodePath to ``parent``.
 */
void scenegraph::NodePath::
reparent_to(NodePath& parent) {
    if (_parent > -1) {
        NodePath& np = *NodePath::_node_paths[_parent];
        np._remove_child(_node_id);
    }
    else if (_tmp_quadtree_entry) {
        _tmp_quadtree_entry.reset(nullptr);
    }
    _parent = parent._node_id;
    parent._insert_child(_node_id);
    _propagate_dirty();
}

/**
 * Return true if the Scene Graph has changed. Finds root NodePath and traverses 
 * the Scene Graph where marked dirty.
 */
bool scenegraph::NodePath::
traverse() {
    NodePath& root = *this;
    while (root._parent != -1) {
        root = *NodePath::_node_paths[root._parent];
    }
    if (!root._dirty) {
        return false;
    }

    if (!root._tmp_quadtree_entry) {
        root._tmp_quadtree_entry.reset(new std::vector<QuadtreeEntry>);
    }
    else {
        root._tmp_quadtree_entry->clear();
    }
    double x_max, x_min, y_max, y_min;
    x_max = y_max = -std::numeric_limits<double>::max();
    x_min = y_min = std::numeric_limits<double>::max();
    SmallList<int> to_process;
    to_process.push_back(root._node_id);
    while(to_process.size() > 0) {
        const int node_path_id = to_process.pop_back();
        NodePath& np = *NodePath::_node_paths[node_path_id];
        if (np._dirty) {
            ChildNodePath& cnp = NodePath::_child_node_paths[np._first_child];
            do {
                to_process.push_back(cnp.node_path_id);
                if (cnp.next != -1) {
                    cnp = NodePath::_child_node_paths[cnp.next];
                }
            } while (cnp.next != -1);
            
            QuadtreeEntry qe;
            qe.node_path_id = np._node_id;
            qe.aabb = np._aabb;
            root._tmp_quadtree_entry->push_back(qe);
            np._update_relative();
            x_max = std::max(x_max, np._aabb.x + np._aabb.hw);
            y_max = std::max(y_max, np._aabb.y + np._aabb.hh);
            x_min = std::min(x_min, np._aabb.x - np._aabb.hw);
            y_min = std::min(y_min, np._aabb.y - np._aabb.hh);
            qe.node_path_id = np._node_id;
            qe.aabb = np._aabb;
            root._tmp_quadtree_entry->push_back(qe);
        }
    }
    double hw = (x_max - x_min) / 2.0, hh = (y_max - y_min) / 2.0;
    AABB qt = AABB(x_min + hw, y_min + hh, hw, hh);
    if (!root._quadtree) {
        root._quadtree.reset(new Quadtree(qt, NodePath::_max_qt_leaf_elements, 
                             NodePath::_max_qt_depth));
    }
    else if (root._quadtree->inside(x_min, y_min) 
            && root._quadtree->inside(x_max, y_max)) {
        if (root._tmp_quadtree_entry->size % 2) {
            std::logic_error("Collected quadtree entries are not a multiple of "
                             "2.");
        }
        while (root._tmp_quadtree_entry->size() > 0) {
            QuadtreeEntry qe_to = root._tmp_quadtree_entry->back();
            root._tmp_quadtree_entry->pop_back();
            QuadtreeEntry qe_from = root._tmp_quadtree_entry->back();
            root._tmp_quadtree_entry->pop_back();
        }
        
    }
    return true;
}

/**
 * Return all NodePath i
 */
SmallList<int> scenegraph::NodePath::
query(AABB& aabb) {

}

/**
 * Hides the NodePath and all it's children.
 */
void scenegraph::NodePath::
hide() {
    if (!_hidden) {
        _hidden = true;
        _propagate_dirty();
    }
}

/**
 * Makes the NodePath visible.
 */
void scenegraph::NodePath::
show() {
    if (_hidden) {
        _hidden = false;
        _propagate_dirty();
    }
}

/**
 * Updates all relative values according to its parent.
 */
void scenegraph::NodePath::
_update_relative() {
    Vector2 relative_pos;
    if (_parent > -1) {
        NodePath& parent = *NodePath::_node_paths[_parent];
        _r_depth = parent._r_depth + _depth;
        _r_angle = parent._r_angle;
        _r_scale = parent._r_scale * _scale;
        relative_pos = *parent._r_position;
    }
    else {
        _r_angle = 0.0;
        _r_scale = _scale;
        _r_depth = _depth;
    }
    
    Vector2 offset = _get_offset();
    Vector2 rotation_center = offset + Vector2(_size.w / 2.0, _size.h / 2.0) 
                              + *_rot_center + *_position;
    Vector2 tr = offset + Vector2(_size.w, 0.0);
    Vector2 bl = offset + Vector2(0.0, _size.h);
    Vector2 br = offset + Vector2(_size.w, _size.h);

    Vector2 rotated_pos;
    if (_r_angle != 0.0) {
        rotated_pos = _position->rotated(_r_angle);
        offset.rotate(_r_angle);
        rotation_center.rotate(_r_angle);
        tr.rotate(_r_angle);
        bl.rotate(_r_angle);
        br.rotate(_r_angle);
    }
    else {
        rotated_pos = Vector2(*_position);
    }
    _r_angle += _angle;

    if (_r_scale != 1.0) {
        if (_distance_relative) {
            rotated_pos[0] *= _r_scale.sx;
            rotated_pos[1] *= _r_scale.sy;
            rotation_center[0] *= _r_scale.sx;
            rotation_center[1] *= _r_scale.sy;
        }
        offset[0] *= _r_scale.sx;
        offset[1] *= _r_scale.sy;
        tr[0] *= _r_scale.sx;
        tr[1] *= _r_scale.sy;
        bl[0] *= _r_scale.sx;
        bl[1] *= _r_scale.sy;
        br[0] *= _r_scale.sx;
        br[1] *= _r_scale.sy;
    }

    relative_pos += rotated_pos + offset; // top-left
    Vector2 tl;
    if (_angle != 0.0) {
        tl = relative_pos - rotation_center;
        tr -= rotation_center;
        bl -= rotation_center;
        br -= rotation_center;

        tl.rotate(_angle);
        tr.rotate(_angle);
        bl.rotate(_angle);
        br.rotate(_angle);

        tl += rotation_center;
        tr += rotation_center;
        bl += rotation_center;
        br += rotation_center;
    }
    else {
        tl = relative_pos;
    }
    *_r_position = tl;
    double x[4], y[4];
    x[0] = tl[0];
    x[1] = tr[0];
    x[2] = bl[0];
    x[3] = br[0];
    y[0] = tl[1];
    y[1] = tr[1];
    y[2] = bl[1];
    y[3] = br[1];
    double x_max, x_min, y_max, y_min;
    x_max = *std::max_element(x, x + 4); 
    y_max = *std::max_element(y, y + 4); 
    x_min = *std::min_element(x, x + 4); 
    y_min = *std::min_element(y, y + 4); 
    _aabb.x = x_min + (x_max - x_min) / 2.0;
    _aabb.y = y_min + (y_max - y_min) / 2.0;
    _aabb.hw = _aabb.x - x_min;
    _aabb.hh = _aabb.y - y_min;
    _dirty = false;
}

/**
 * Updates the AABB of the NodePath. This needs to be called after every call to
 * _update_relative().
 */
void scenegraph::NodePath::
_update_aabb() {
    Vector2 relative_pos;
    double base_angle = 0.0;
    if (_parent > -1) {
        NodePath& parent = *NodePath::_node_paths[_parent];
        relative_pos = *parent._r_position;
        base_angle = parent._r_angle;
    }
}

/**
 * Return Vector2 with origin offset relative to original size.
 */
scenegraph::Vector2 scenegraph::NodePath::
_get_offset() {
    Vector2 offset;
    switch (_origin) {
        case TOP_LEFT: {
            break;
        }
        case TOP_CENTER: {
            offset[0] = -_size.w / 2.0;
            offset[1] = 0.0;
            break;
        }
        case TOP_RIGHT: {
            offset[0] = -_size.w;
            offset[1] = 0.0;
            break;
        }
        case CENTER_LEFT: {
            offset[0] = 0.0;
            offset[1] = -_size.h / 2.0;
            break;
        }
        case CENTER: {
            offset[0] = -_size.w / 2.0;
            offset[1] = -_size.h / 2.0;
            break;
        }
        case CENTER_RIGHT: {
            offset[0] = -_size.w;
            offset[1] = -_size.h / 2.0;
            break;
        }
        case BOTTOM_LEFT: {
            offset[0] = 0.0;
            offset[1] = -_size.h;
            break;
        }
        case BOTTOM_CENTER: {
            offset[0] = -_size.w / 2.0;
            offset[1] = -_size.h;
            break;
        }
        case BOTTOM_RIGHT: {
            offset[0] = -_size.w;
            offset[1] = -_size.h;
            break;
        }
    }
    return offset;
}

/**
 * Gets the offset to the rotation center in relation to the origin.
 */
scenegraph::Vector2 scenegraph::NodePath::
_get_rotation_center_offset() {
    return _get_offset() + Vector2(_size.w / 2.0, _size.h / 2.0) + *_rot_center;
}

/**
 * Insert NodePath with ``node_path_id`` as child.
 */
void scenegraph::NodePath::
_insert_child(const int node_path_id) {
    ChildNodePath cnp;
    cnp.node_path_id = node_path_id;
    const int cnp_id = NodePath::_child_node_paths.insert(cnp);
    if (_first_child == -1) {
        _first_child = cnp_id;
    }
    else {
        ChildNodePath& search_cnp = NodePath::_child_node_paths[_first_child];
        while (search_cnp.next != -1) {
            search_cnp = NodePath::_child_node_paths[search_cnp.next];
        }
        search_cnp.next = cnp_id;
    }
    _propagate_dirty();
}

/**
 * Remove NodePath with ``node_path_id`` from children.
 */
void scenegraph::NodePath::
_remove_child(const int node_path_id) {
    ChildNodePath& cnp = NodePath::_child_node_paths[_first_child];
    int cnp_id = _first_child;
    if (cnp.node_path_id == node_path_id) {
        _first_child = cnp.next;
    }
    else {
        while (NodePath::_child_node_paths[cnp.next].node_path_id != node_path_id) {
            cnp = NodePath::_child_node_paths[cnp.next];
            cnp_id = cnp.next;
        }
        cnp.next = NodePath::_child_node_paths[cnp.next].next;
    }
    NodePath::_child_node_paths.erase(cnp_id);
}

/**
 * If necessary, propagates the _dirty flag to all descendants and along the 
 * direct path to the root NodePath. 
 */
void scenegraph::NodePath::
_propagate_dirty() {
    if (!_dirty) {
        if (_parent > -1) {
            NodePath& np = *NodePath::_node_paths[_parent];
            do {
                np._dirty = true;
                if (np._parent > -1) {
                    np = *NodePath::_node_paths[np._parent];
                }
            } while (np._parent > -1);
        }
    }
    if (_first_child > -1) {
        SmallList<int> to_process;
        to_process.push_back(_first_child);
        while (to_process.size() > 0) {
            const int cnp_id = to_process.pop_back();
            ChildNodePath& cnp = NodePath::_child_node_paths[cnp_id];
            NodePath& np = *NodePath::_node_paths[cnp.node_path_id];
            if (cnp.next > -1) {
                to_process.push_back(cnp.next);
            }
            if (np._first_child > -1) {
                to_process.push_back(np._first_child);
            }
            np._dirty = true;
        }
    }
    _dirty = true;
}

/**
 * 
 */
int scenegraph::NodePath::
get_id() {
    return _node_id;
}

/**
 * Set both parts of position to ``v``.
 */
void scenegraph::NodePath::
set_pos(const double v) {
    if (!_position) {
        _position.reset(new Vector2(v));
        _propagate_dirty();
    }
    else {
        Vector2& vec = *_position;
        if (vec[0] != v || vec[1] != v) {
            vec[0] = v;
            vec[1] = v;
            _propagate_dirty();
        }
    }
}

/**
 * Set position to ``x``, ``y``.
 */
void scenegraph::NodePath::
set_pos(const double x, const double y) {
   if (!_position) {
        _position.reset(new Vector2(x, y));
        _propagate_dirty();
    }
    else {
        Vector2& vec = *_position;
        if (vec[0] != x || vec[1] != y) {
            vec[0] = x;
            vec[1] = y;
            _propagate_dirty();
        }
    }
}

/**
 * Set position to ``p``.
 */
void scenegraph::NodePath::
set_pos(Vector2& p) {
   if (!_position) {
        _position.reset(new Vector2(p));
        _propagate_dirty();
    }
    else {
        Vector2& vec = *_position;
        if (vec != p) {
            vec[0] = p[0];
            vec[1] = p[1];
            _propagate_dirty();
        }
    }
}

/**
 * 
 */
scenegraph::Vector2 scenegraph::NodePath::
get_pos() {
    return *_position;
}

/**
 * Set scale to ``s``.
 */
void scenegraph::NodePath::
set_scale(const double s) {
    if (s <= 0.0) {
        throw std::domain_error("Scale must be positive.");
    }
    if (_scale.sx != s || _scale.sy != s) {
        _scale.sx = _scale.sy = s;
        _propagate_dirty();
    }
}

/**
 * Set scale to ``sx``, ``sy``.
 */
void scenegraph::NodePath::
set_scale(const double sx, const double sy) {
    if (sx <= 0.0 || sy <= 0.0) {
        throw std::domain_error("Scale must be positive.");
    }
    if (_scale.sx != sx || _scale.sy != sy) {
        _scale.sx = sx;
        _scale.sy = sy;
        _propagate_dirty();
    }
}

/**
 * Set scale to ``s``.
 */
void scenegraph::NodePath::
set_scale(const Scale& s) {
    if (s.sx <= 0.0 || s.sy <= 0.0) {
        throw std::domain_error("Scale must be positive.");
    }
    if (_scale.sx != s.sx || _scale.sy != s.sy) {
        _scale.sx = s.sx;
        _scale.sy = s.sy;
        _propagate_dirty();
    }
}

/**
 * 
 */
scenegraph::Scale scenegraph::NodePath::
get_scale() {
    return _scale;
}

/**
 * Set angle to ``a`` degrees if ``radians`` is false (default) otherwise
 * to ``a`` radians.
 */
void scenegraph::NodePath::
set_angle(double a, bool radians) {
    if (radians) {
        a = a * to_deg;
    }
    if (a != _angle) {
        _angle = a;
        _propagate_dirty();
    }
}

/**
 * Get angle in degrees if ``radians`` is false, otherwise in radians.
 */
double scenegraph::NodePath::
get_angle(bool radians) {
    if (radians) {
        return _angle * to_rad;
    }
    else {
        return _angle;
    }
}

/**
 * 
 */
void scenegraph::NodePath::
set_rotation_center(const double x, const double y) {
    if (!_rot_center) {
        _rot_center.reset(new Vector2(x, y));
        _propagate_dirty();
    }
    else {
        Vector2& vec = *_rot_center;
        if (vec[0] != x || vec[1] != y) {
            vec[0] = x;
            vec[1] = y;
            _propagate_dirty();
        }
    }
}

/**
 * 
 */
void scenegraph::NodePath::
set_rotation_center(Vector2& c) {
    if (!_rot_center) {
        _rot_center.reset(new Vector2(c));
        _propagate_dirty();
    }
    else {
        Vector2& vec = *_rot_center;
        if (vec != c) {
            vec[0] = c[0];
            vec[1] = c[1];
            _propagate_dirty();
        }
    }
}

/**
 * 
 */
scenegraph::Vector2 scenegraph::NodePath::
get_rotation_center() {
    return *_rot_center;
}

/**
 * Set depth to ``d``.
 */
void scenegraph::NodePath::
set_depth(const int d) {
    if (_depth != d) {
        _depth = d;
        _propagate_dirty();
    }
}

/**
 * 
 */
int scenegraph::NodePath::
get_depth() {
    return _depth;
}

/**
 * Set origin to ``o``.
 */
void scenegraph::NodePath::
set_origin(Origin o) {
    if (_origin != o) {
        _origin = o;
        _propagate_dirty();  // Necessary?
    }
}

/**
 * 
 */
scenegraph::Origin scenegraph::NodePath::
get_origin() {
    return _origin;
}

/**
 * 
 */
scenegraph::Vector2 scenegraph::NodePath::
get_relative_pos() {
    return *_r_position;
}

/**
 * 
 */
scenegraph::Scale scenegraph::NodePath::
get_relative_scale() {
    return _r_scale;
}

/**
 * 
 */
double scenegraph::NodePath::
get_relative_angle() {
    return _r_angle;
}

/**
 * 
 */
int scenegraph::NodePath::
get_relative_depth() {
    return _r_depth;
}

/**
 * Set the represented size in world space of NodePath.
 */
void scenegraph::NodePath::
set_size(const Size& s) {
    if (s.w >= 0.0 && s.h >= 0.0) {
        _size = s;
    }
    else {
        throw std::domain_error("Size must be positive.");
    }
}

/**
 * Set the represented size in world space of NodePath.
 */
void scenegraph::NodePath::
set_size(const double w, const double h) {
    if (w >= 0.0 && h >= 0.0) {
        _size.w = w;
        _size.h = h;
    }
    else {
        throw std::domain_error("Size must be positive.");
    }
}

/**
 * 
 */
scenegraph::Size scenegraph::NodePath::
get_size() {
    return _size;
}

/**
 * 
 */
void scenegraph::NodePath::
set_distance_relative(const bool v) {
    if (v != _distance_relative) {
        _distance_relative = v;
        _propagate_dirty();
    }
}

/**
 * 
 */
bool scenegraph::NodePath::
get_distance_relative() {
    return _distance_relative;
}
