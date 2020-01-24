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

#include <algorithm>
#include <stdexcept>
#include <limits>
#include <iostream>


/**
 * NodeData Struct
 * Holds all relevant data for a Node to avoid complicated memory handling.
 */


scenegraph::NodeData::
NodeData() {
    _dirty = true;
    _isnew = true;
    _distance_relative = false;
    _hidden = false;
    _parent = -1;
    _first_child = -1;
    _angle = _r_angle = 0.0;
    _depth = _r_depth = 1;
    _origin = TOP_LEFT;
    _ref_count = 1;
}

/* NodeData */
scenegraph::NodeData::
~NodeData() {

}

/**
 *
 */
bool scenegraph::NodeData::
_traverse(SceneGraphDataHandler& sgdh) {
    int root_node_id = _node_data_id;
    while (sgdh._nd[root_node_id]->_parent > -1) {
        root_node_id = sgdh._nd[root_node_id]->_parent;
    }
    NodeData& root = *sgdh._nd[root_node_id];
    if (!root._dirty) {
        return false;
    }

    root._tmp_quadtree_entry.clear();
    double x_max, x_min, y_max, y_min;
    x_max = y_max = -std::numeric_limits<double>::max();
    x_min = y_min = std::numeric_limits<double>::max();
    SmallList<int> to_process;
    to_process.push_back(root._node_data_id);
    while(to_process.size() > 0) {
        const int node_id = to_process.pop_back();
        NodeData& nd = *sgdh._nd[node_id];
        if (nd._dirty) {
            int child_id = nd._first_child;
            while (child_id != -1) {
                ChildNode& cn = sgdh._child_nodes[child_id];
                to_process.push_back(cn.node_id);
                child_id = cn.next;
            }

            QuadtreeEntry qe;
            qe.node_id = (nd._isnew) ? -1 : nd._node_data_id;
            qe.aabb = nd._aabb;
            root._tmp_quadtree_entry.push_back(qe);
            nd._update_relative(sgdh);
            x_max = std::max(x_max, nd._aabb.x + nd._aabb.hw);
            y_max = std::max(y_max, nd._aabb.y + nd._aabb.hh);
            x_min = std::min(x_min, nd._aabb.x - nd._aabb.hw);
            y_min = std::min(y_min, nd._aabb.y - nd._aabb.hh);
            qe.node_id = nd._node_data_id;
            qe.aabb = nd._aabb;
            root._tmp_quadtree_entry.push_back(qe);
        }
    }

    double hw = (x_max - x_min) / 2.0, hh = (y_max - y_min) / 2.0;
    AABB qt = AABB(x_min + hw, y_min + hh, hw, hh);
    if (!root._quadtree) {
        root._quadtree.reset(new Quadtree(qt, NodeData::_max_qt_leaf_elements,
                             NodeData::_max_qt_depth));
    }
    else if (!root._quadtree->inside(x_min, y_min)
            || !root._quadtree->inside(x_max, y_max)) {
        root._quadtree->resize(qt);
    }

    if (root._tmp_quadtree_entry.size() % 2) {
        throw std::logic_error("Collected quadtree entries are not a multiple "
                               "of 2.");
    }
    while (root._tmp_quadtree_entry.size() > 0) {
        QuadtreeEntry& qe_to = *(root._tmp_quadtree_entry.end() - 1);
        QuadtreeEntry& qe_from = *(root._tmp_quadtree_entry.end() - 2);
        if (qe_from.node_id == -1) {
            root._quadtree->insert(qe_to.node_id, qe_to.aabb);
        }
        else if (qe_from.aabb != qe_to.aabb) {
            root._quadtree->move(qe_from.node_id, qe_from.aabb, qe_to.aabb);
        }
        root._tmp_quadtree_entry.pop_back();
        root._tmp_quadtree_entry.pop_back();
    }
    root._quadtree->cleanup();
    return true;
}

/**
 *
 */
SmallList<int> scenegraph::NodeData::
_query(SceneGraphDataHandler& sgdh, AABB& aabb, const bool depth_sorted) {
    int root_node_id = _node_data_id;
    while (sgdh._nd[root_node_id]->_parent > -1) {
        root_node_id = sgdh._nd[root_node_id]->_parent;
    }
    NodeData& root = *sgdh._nd[root_node_id];
    if (!root._quadtree) {
        throw std::logic_error("Quadtree not built yet");
    }
    SmallList<int> result, qt_result = root._quadtree->query(aabb);
    while (qt_result.size() > 0) {
        const int node_id = qt_result.pop_back();
        NodeData& nd = *sgdh._nd[node_id];
        if (!nd._hidden) {
            result.push_back(node_id);
        }
    }
    if (!depth_sorted) {
        return result;
    }

    // TODO: learn how to implement Random Access Iterator!!!
    std::vector<DepthData> v;
    v.reserve(result.size());
    for (auto i=0; i < result.size(); ++i) {
        v.push_back(DepthData(result[i], sgdh._nd[result[i]]->_r_depth));
    }
    result.clear();
    std::sort(v.begin(), v.end(), comp_depth_data);
    for (auto& it : v) {
        result.push_back(it.node_id);
    }
    return result;
}

/**
 * Insert Node with ``node_id`` as child.
 */
void scenegraph::NodeData::
_insert_child(SceneGraphDataHandler& sgdh, const int node_id) {
    ChildNode cnp;
    cnp.node_id = node_id;
    const int cnp_id = sgdh._child_nodes.insert(cnp);
    if (_first_child == -1) {
        _first_child = cnp_id;
    }
    else {
        int current = _first_child;
        int next = sgdh._child_nodes[current].next;
        while (next != -1) {
            current = next;
            next = sgdh._child_nodes[next].next;
        }
        sgdh._child_nodes[current].next = cnp_id;
    }
    _propagate_dirty(sgdh);
}

/**
 * Remove Node with ``node_id`` from children.
 */
void scenegraph::NodeData::
_remove_child(SceneGraphDataHandler& sgdh, const int node_id) {
    if (_first_child == -1) {
        throw std::range_error("No children");
    }
    ExtFreeList<ChildNode>& cnps = sgdh._child_nodes;
    if (!cnps.active(_first_child)) {
        throw std::range_error("Child is not present");
    }
    ChildNode& cnp = cnps[_first_child];
    int cnp_id = _first_child;
    if (cnp.node_id == node_id) {
        _first_child = cnp.next;
    }
    else {
        if (cnp.next == -1 || !cnps.active(cnp.next)) {
            throw std::range_error("Child is not present");
        }
        while (cnps[cnp.next].node_id != node_id) {
            if (cnp.next == -1 || !cnps.active(cnp.next)) {
                throw std::range_error("Child not found");
            }
            cnp = cnps[cnp.next];
            cnp_id = cnp.next;
        }
        cnp.next = cnps[cnp.next].next;
    }
    cnps.erase(cnp_id);
}

/**
 * If necessary, propagates the _dirty flag to all descendants and along the
 * direct path to the root Node.
 */
void scenegraph::NodeData::
_propagate_dirty(SceneGraphDataHandler& sgdh) {
    if (_parent > -1) {
        int parent = _parent;
        while (parent > -1) {
            NodeData& nd = *sgdh._nd[parent];
            nd._dirty = true;
            parent = nd._parent;
        }
    }
    if (_first_child > -1) {
        SmallList<int> to_process;
        to_process.push_back(_first_child);
        while (to_process.size() > 0) {
            const int cnp_id = to_process.pop_back();
            ChildNode& cnp = sgdh._child_nodes[cnp_id];
            NodeData& nd = *sgdh._nd[cnp.node_id];
            if (cnp.next > -1) {
                to_process.push_back(cnp.next);
            }
            if (nd._first_child > -1) {
                to_process.push_back(nd._first_child);
            }
            nd._dirty = true;
        }
    }
    _dirty = true;
}

/**
 * Updates all relative values according to its parent.
 */
void scenegraph::NodeData::
_update_relative(SceneGraphDataHandler& sgdh) {
    Vector2 local_origin;  // origin of local coordinate system
    Scale base_scale;
    double base_angle;
    bool base_dist_rel;
    if (_parent > -1) {
        NodeData& parent = *sgdh._nd[_parent];
        local_origin = parent._r_position;
        base_scale = parent._r_scale;
        base_angle = parent._r_angle;
        _r_depth = parent._r_depth + _depth;
        base_dist_rel = parent._distance_relative || _distance_relative;
    }
    else {
        local_origin = _get_offset();
        base_scale = Scale();
        base_angle = 0.0;
        _r_depth = _depth;
        base_dist_rel = _distance_relative;
    }

    Vector2 position = _position;
    Vector2 tl = _get_offset();
    Vector2 tr = tl + Vector2(_size.w, 0.0);
    Vector2 bl = tl + Vector2(0.0, _size.h);
    Vector2 br = tl + Vector2(_size.w, _size.h);
    Vector2 rotation_center = tl + Vector2(_size.w / 2.0, _size.h / 2.0)
                              + _rot_center;

    if (base_scale != 1.0 || _scale != 1.0) {
        double sx = base_scale.sx, sy = base_scale.sy;
        if (_scale != 1.0) {
            sx *= _scale.sx;
            sy *= _scale.sy;
        }
        if (base_dist_rel) {
            position[0] *= sx;
            position[1] *= sy;
        }
        tl[0] *= sx;
        tl[1] *= sy;
        tr[0] *= sx;
        tr[1] *= sy;
        bl[0] *= sx;
        bl[1] *= sy;
        br[0] *= sx;
        br[1] *= sy;
        rotation_center[0] *= sx;
        rotation_center[1] *= sy;
    }

    if (base_angle != 0.0) {
        position.rotate(base_angle);
        rotation_center.rotate(base_angle);
        tl.rotate(base_angle);
        tr.rotate(base_angle);
        bl.rotate(base_angle);
        br.rotate(base_angle);
    }

    _r_position = local_origin + position;
    _tl_position = _r_position + tl;
    _r_scale = base_scale * _scale;
    _r_angle = base_angle + _angle;
    _r_size = _size * _r_scale;

    rotation_center += position;
    tl += position;
    tr += position;
    bl += position;
    br += position;

    if (_angle != 0.0) {
        tl -= rotation_center;
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

    tl += local_origin;
    tr += local_origin;
    bl += local_origin;
    br += local_origin;

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
    _dirty = _isnew = false;
}

/**
 * Return Vector2 with origin offset relative to original size.
 */
Vector2 scenegraph::NodeData::
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
 * Node Class
 * Serves as a wrapper around NodeData. Multiple copies of NodePath can access
 * a single instance of NodeData without protection against race conditions if
 * being accessed in parallel.
 */

/**
 * Default Constructor.
 */
scenegraph::Node::
Node(SceneGraphDataHandler& sgdh) : _sgdh(sgdh) {
    _node_id = _sgdh._nd.insert(new NodeData());
    _sgdh._nd[_node_id]->_node_data_id = _node_id;
}

/**
 * Destructor, clean up relations.
 */
scenegraph::Node::
~Node() {
    if (_node_id > -1) {
        NodeData& nd = _get_node_data(_node_id);
        if (nd._ref_count == 1) {
            /* If ours is the last reference, delete the NodeData */
            _sgdh._nd.erase(_node_id);
        }
        else {
            --nd._ref_count;
        }
    }
}

/**
 * Copy Constructor. Creates a copy of Node ``other`` with access to the same
 * NodeData instance.
 */
scenegraph::Node::
Node(const Node& other) : _sgdh(other._sgdh) {
    _node_id = other._node_id;
    NodeData& nd = _get_node_data(_node_id);
    ++nd._ref_count;
}

/**
 * Move Constructor. Invalidates ``other``.
 */
scenegraph::Node::
Node(Node&& other) noexcept  : _sgdh(other._sgdh) {
    _node_id = other._node_id;
    other._node_id = -1;
}

/**
 * Copy Assignment Operator. Creates a copy of Node ``other`` with access to the
 * same NodeData instance.
 */
scenegraph::Node& scenegraph::Node::
operator=(const Node& other) {
    if (other._sgdh._nd.active(_node_id)) {
        NodeData& nd = _get_node_data(_node_id);
        --nd._ref_count;
    }
    _node_id = other._node_id;
    _sgdh = other._sgdh;
    NodeData& nd = _get_node_data(_node_id);
    ++nd._ref_count;
    return *this;
}

/**
 * Move Assignment Operator. Invalidates ``other``. TODO: This doesn't seem right!
 */
scenegraph::Node& scenegraph::Node::
operator=(Node&& other) noexcept {
    if (other._sgdh._nd.active(_node_id)) {
        NodeData& nd = _get_node_data(_node_id);
        --nd._ref_count;
    }
    _node_id = other._node_id;
    _sgdh = other._sgdh;
    NodeData& nd = _get_node_data(_node_id);
    ++nd._ref_count;
    return *this;
}

/**
 * Attach new child Node to this.
 */
scenegraph::Node scenegraph::Node::
attach_node() {
    Node n = {_sgdh};
    NodeData& cnd = _get_node_data(n._node_id);
    NodeData& tnd = _get_node_data(_node_id);
    cnd._parent = _node_id;
    cnd._distance_relative = tnd._distance_relative;
    cnd._origin = tnd._origin;
    tnd._insert_child(_sgdh, n._node_id);
    tnd._propagate_dirty(_sgdh);
    return n;
}

/**
 * Reparents this Node to ``parent``.
 */
void scenegraph::Node::
reparent_to(Node& parent) {
    reparent_to(parent._node_id);
}

/**
 * Reparents this Node to ``parent``.
 */
void scenegraph::Node::
reparent_to(const int parent) {
    int _p_id = _sgdh._nd[parent]->_parent;
    while (_p_id > -1) {
        if (_p_id == _node_id) {
            throw std::logic_error("Unable to reparent: new parent is a child "
                                   "of this Node.");
        }
        NodeData& lnd = _get_node_data(_p_id);
        _p_id = lnd._parent;
    }

    NodeData& nd = _get_node_data(_node_id);
    if (nd._parent > -1) {
        NodeData& pnd = *_sgdh._nd[nd._parent];
        pnd._remove_child(_sgdh, _node_id);
    }
    nd._tmp_quadtree_entry.clear();

    nd._parent = parent;
    NodeData& pnd = *_sgdh._nd[nd._parent];
    pnd._insert_child(_sgdh, _node_id);
    nd._propagate_dirty(_sgdh);
}

/**
 * Return true if the Scene Graph has changed. Finds root Node and traverses
 * the Scene Graph where marked dirty.
 */
bool scenegraph::Node::
traverse() {
    return _get_node_data(_node_id)._traverse(_sgdh);
}

/**
 * Return all Node indices that are visible and overlap with ``aabb`` since
 * the last call to traverse().
 */
SmallList<int> scenegraph::Node::
query(AABB& aabb, const bool depth_sorted) {
    return _get_node_data(_node_id)._query(_sgdh, aabb, depth_sorted);
}

/**
 * Hides the Node and all it's children.
 */
void scenegraph::Node::
hide() {
    NodeData& nd = _get_node_data(_node_id);
    if (!nd._hidden) {
        nd._hidden = true;
        nd._propagate_dirty(_sgdh);
    }
}

/**
 * Makes the Node visible.
 */
void scenegraph::Node::
show() {
    NodeData& nd = _get_node_data(_node_id);
    if (nd._hidden) {
        nd._hidden = false;
        nd._propagate_dirty(_sgdh);
    }
}

/**
 * Return the root NodeData reference.
 */
inline scenegraph::NodeData& scenegraph::Node::
_get_root() {
    if (!_sgdh._nd.active(_node_id)) {
        throw std::logic_error("Tried to access invalid NodeData");
    }
    int search_id = _node_id;
    while (_sgdh._nd[search_id]->_parent > -1) {
        search_id = _sgdh._nd[search_id]->_parent;
    }
    return *_sgdh._nd[search_id];
}

/**
 * Return the NodeData reference of this.
 */
inline scenegraph::NodeData& scenegraph::Node::
_get_node_data(const int node_id) {
    if (!_sgdh._nd.active(node_id)) {
        throw std::logic_error("Tried to access invalid NodeData");
    }
    return *_sgdh._nd[node_id];
}

/**
 * Traverses the scene_graph if either ``node_a`` or ``node_b`` is marked dirty.
 */
inline void scenegraph::Node::
_check_dirty(Node& node_a, Node& node_b) {
    NodeData& a = _get_node_data(node_a._node_id);
    NodeData& b = _get_node_data(node_b._node_id);
    if (a._dirty || b._dirty) {
        traverse();
    }
}

/**
 * Return the Node index, useful to later retrieve a reference by id.
 */
int scenegraph::Node::
get_id() {
    return _node_id;
}

/**
 * Set both parts of position to ``v``.
 */
void scenegraph::Node::
set_pos(const double v) {
    NodeData& nd = _get_node_data(_node_id);
    if (nd._position[0] != v || nd._position[1] != v) {
        nd._position[0] = v;
        nd._position[1] = v;
        nd._propagate_dirty(_sgdh);
    }
}

/**
 * Set both parts of position to ``v``, relative to ``other``. If either this
 * or ``other`` is marked dirty, triggers scenegraph traversal to update the
 * relative positions.
 */
void scenegraph::Node::
set_pos(Node& other, const double v) {
    _check_dirty(*this, other);
    Vector2 offset = other.get_relative_pos() - get_relative_pos() + v;
    offset += get_pos();
    set_pos(offset);
}

/**
 * Set position to ``x``, ``y``.
 */
void scenegraph::Node::
set_pos(const double x, const double y) {
    NodeData& nd = _get_node_data(_node_id);
    if (nd._position[0] != x || nd._position[1] != y) {
        nd._position[0] = x;
        nd._position[1] = y;
        nd._propagate_dirty(_sgdh);
    }
}

/**
 * Set position to ``x``, ``y``, relative to ``other``. If either this
 * or ``other`` is marked dirty, triggers scenegraph traversal to update the
 * relative positions.
 */
void scenegraph::Node::
set_pos(Node& other, const double x, const double y) {
    _check_dirty(*this, other);
    Vector2 offset = other.get_relative_pos() - get_relative_pos();
    offset += Vector2(x, y);
    offset += get_pos();
    set_pos(offset);
}

/**
 * Set position to ``p``.
 */
void scenegraph::Node::
set_pos(Vector2& p) {
    NodeData& nd = _get_node_data(_node_id);
    if (nd._position != p) {
        nd._position = p;
        nd._propagate_dirty(_sgdh);
    }
}

/**
 * Set position to ``x``, ``y``, relative to ``other``. If either this or
 * ``other`` is marked dirty, triggers scenegraph traversal to update the
 * relative positions.
 */
void scenegraph::Node::
set_pos(Node& other, Vector2& p) {
    _check_dirty(*this, other);
    Vector2 offset = other.get_relative_pos() - get_relative_pos() + p;
    offset += get_pos();
    set_pos(offset);
}

/**
 * Return a ``Vector2`` of the current position.
 */
Vector2 scenegraph::Node::
get_pos() {
    return _get_node_data(_node_id)._position;
}

/**
 * Return a ``Vector2`` of the current position, relative to ``other``.
 */
Vector2 scenegraph::Node::
get_pos(Node& other) {
    _check_dirty(*this, other);
    return get_relative_pos() - other.get_relative_pos();
}

/**
 * Set scale to ``s``.
 */
void scenegraph::Node::
set_scale(const double s) {
    if (s <= 0.0) {
        throw std::domain_error("Scale must be positive.");
    }
    NodeData& nd = _get_node_data(_node_id);
    if (nd._scale.sx != s || nd._scale.sy != s) {
        nd._scale.sx = nd._scale.sy = s;
        nd._propagate_dirty(_sgdh);
    }
}

/**
 * Set scale to ``s``, relative to ``other``.
 */
void scenegraph::Node::
set_scale(Node& other, const double s) {
    if (s <= 0.0) {
        throw std::domain_error("Scale must be positive.");
    }
    _check_dirty(*this, other);
    Scale t_rel = get_relative_scale();
    Scale t_s = get_scale();
    Scale o_rel = other.get_relative_scale();
    set_scale(
        t_s.sx / o_rel.sx * s / t_rel.sx,
        t_s.sy / o_rel.sy * s / t_rel.sy
    );
}

/**
 * Set scale to ``sx``, ``sy``.
 */
void scenegraph::Node::
set_scale(const double sx, const double sy) {
    if (sx <= 0.0 || sy <= 0.0) {
        throw std::domain_error("Scale must be positive.");
    }
    NodeData& nd = _get_node_data(_node_id);
    if (nd._scale.sx != sx || nd._scale.sy != sy) {
        nd._scale.sx = sx;
        nd._scale.sy = sy;
        nd._propagate_dirty(_sgdh);
    }
}

/**
 * Set scale to ``sx``, ``sy``, relative to ``other``.
 */
void scenegraph::Node::
set_scale(Node& other, const double sx, const double sy) {
    if (sx <= 0.0 || sy <= 0.0) {
        throw std::domain_error("Scale must be positive.");
    }
    _check_dirty(*this, other);
    Scale t_rel = get_relative_scale();
    Scale t_s = get_scale();
    Scale o_rel = other.get_relative_scale();
    set_scale(
        t_s.sx / o_rel.sx * sx / t_rel.sx,
        t_s.sy / o_rel.sy * sy / t_rel.sy
    );
}

/**
 * Set scale to ``s``.
 */
void scenegraph::Node::
set_scale(const Scale& s) {
    if (s.sx <= 0.0 || s.sy <= 0.0) {
        throw std::domain_error("Scale must be positive.");
    }
    NodeData& nd = _get_node_data(_node_id);
    if (nd._scale.sx != s.sx || nd._scale.sy != s.sy) {
        nd._scale.sx = s.sx;
        nd._scale.sy = s.sy;
        nd._propagate_dirty(_sgdh);
    }
}

/**
 * Set scale to ``s``, relative to ``other``.
 */
void scenegraph::Node::
set_scale(Node& other, const Scale& s) {
    if (s.sx <= 0.0 || s.sy <= 0.0) {
        throw std::domain_error("Scale must be positive.");
    }
    _check_dirty(*this, other);
    Scale t_rel = get_relative_scale();
    Scale t_s = get_scale();
    Scale o_rel = other.get_relative_scale();
    set_scale(
        t_s.sx / o_rel.sx * s.sx / t_rel.sx,
        t_s.sy / o_rel.sy * s.sy / t_rel.sy
    );
}

/**
 *
 */
scenegraph::Scale scenegraph::Node::
get_scale() {
    return _get_node_data(_node_id)._scale;
}

/**
 *
 */
scenegraph::Scale scenegraph::Node::
get_scale(Node& other) {
    _check_dirty(*this, other);
    Scale t_rel = get_relative_scale();
    Scale o_rel = other.get_relative_scale();
    Scale res;
    res.sx = t_rel.sx / o_rel.sx;
    res.sy = t_rel.sy / o_rel.sy;
    return res;
}

/**
 * Set angle to ``a`` degrees if ``radians`` is false (default) otherwise
 * to ``a`` radians.
 */
void scenegraph::Node::
set_angle(double a, bool radians) {
    if (radians) {
        a = a * to_deg;
    }
    NodeData& nd = _get_node_data(_node_id);
    if (a != nd._angle) {
        // nd._angle = clamp_angle(a);
        nd._angle = a;
        nd._propagate_dirty(_sgdh);
    }
}

/**
 * Set angle to ``a`` degrees if ``radians`` is false (default) otherwise
 * to ``a`` radians, relative to ``other``.
 */
void scenegraph::Node::
set_angle(Node& other, double a, bool radians) {
    if (radians) {
        a = a * to_deg;
    }
    // a = clamp_angle(a);
    _check_dirty(*this, other);
    double rel_a = other.get_relative_angle() + a - get_angle()
                   + get_relative_angle();
    // set_angle(clamp_angle(rel_a), false);
    set_angle(rel_a, false);
}

/**
 * Get angle in degrees if ``radians`` is false, otherwise in radians.
 */
double scenegraph::Node::
get_angle(bool radians) {
    NodeData& nd = _get_node_data(_node_id);
    if (radians) {
        return nd._angle * to_rad;
    }
    else {
        return nd._angle;
    }
}

/**
 * Get angle in degrees if ``radians`` is false, otherwise in radians, relative
 * to ``other``.
 */
double scenegraph::Node::
get_angle(Node& other, bool radians) {
    _check_dirty(*this, other);
    double a_rel = other.get_relative_angle() - get_relative_angle();
    // a_rel = clamp_angle(a_rel);

    if (radians) {
        return a_rel * to_rad;
    }
    else {
        return a_rel;
    }
}

/**
 *
 */
void scenegraph::Node::
set_rotation_center(const double x, const double y) {
    NodeData& nd = _get_node_data(_node_id);
    if (nd._rot_center[0] != x || nd._rot_center[1] != y) {
        nd._rot_center[0] = x;
        nd._rot_center[1] = y;
        nd._propagate_dirty(_sgdh);
    }
}

/**
 *
 */
void scenegraph::Node::
set_rotation_center(Vector2& c) {
    NodeData& nd = _get_node_data(_node_id);
    if (nd._rot_center != c) {
        nd._rot_center[0] = c[0];
        nd._rot_center[1] = c[1];
        nd._propagate_dirty(_sgdh);
    }
}

/**
 *
 */
Vector2 scenegraph::Node::
get_rotation_center() {
    return _get_node_data(_node_id)._rot_center;
}

/**
 * Set depth to ``d``.
 */
void scenegraph::Node::
set_depth(const int d) {
    NodeData& nd = _get_node_data(_node_id);
    if (nd._depth != d) {
        nd._depth = d;
        nd._propagate_dirty(_sgdh);
    }
}

/**
 * Set depth to ``d``, relative to ``other``.
 */
void scenegraph::Node::
set_depth(Node& other, const int d) {
    _check_dirty(*this, other);
    set_depth(
        get_relative_depth() -  get_depth() - other.get_relative_depth() + d);
}

/**
 *
 */
int scenegraph::Node::
get_depth() {
    return _get_node_data(_node_id)._depth;
}

/**
 *
 */
int scenegraph::Node::
get_depth(Node& other) {
    _check_dirty(*this, other);
    return get_relative_depth() - other.get_relative_depth();
}

/**
 * Set origin to ``o``.
 */
void scenegraph::Node::
set_origin(Origin o) {
    NodeData& nd = _get_node_data(_node_id);
    if (nd._origin != o) {
        nd._origin = o;
        nd._propagate_dirty(_sgdh);  // Necessary?
    }
}

/**
 *
 */
scenegraph::Origin scenegraph::Node::
get_origin() {
    return _get_node_data(_node_id)._origin;
}

/**
 *
 */
Vector2 scenegraph::Node::
get_relative_pos() {
    return _get_node_data(_node_id)._r_position;
}

/**
 *
 */
scenegraph::Scale scenegraph::Node::
get_relative_scale() {
    return _get_node_data(_node_id)._r_scale;
}

/**
 *
 */
double scenegraph::Node::
get_relative_angle() {
    return _get_node_data(_node_id)._r_angle;
}

/**
 *
 */
int scenegraph::Node::
get_relative_depth() {
    return _get_node_data(_node_id)._r_depth;
}

/**
 * Return the relative size.
 */
scenegraph::Size scenegraph::Node::
get_relative_size() {
    return _get_node_data(_node_id)._r_size;
}

/**
 * Set the represented size in world space of Node.
 */
void scenegraph::Node::
set_size(const Size& s) {
    NodeData& nd = _get_node_data(_node_id);
    if (s.w >= 0.0 && s.h >= 0.0) {
        nd._size = s;
        nd._propagate_dirty(_sgdh);
    }
    else {
        throw std::domain_error("Size must be positive.");
    }
}

/**
 * Set the represented size in world space of Node.
 */
void scenegraph::Node::
set_size(const double w, const double h) {
    NodeData& nd = _get_node_data(_node_id);
    if (w >= 0.0 && h >= 0.0) {
        nd._size.w = w;
        nd._size.h = h;
        nd._propagate_dirty(_sgdh);
    }
    else {
        throw std::domain_error("Size must be positive.");
    }
}

/**
 *
 */
scenegraph::Size scenegraph::Node::
get_size() {
    return _get_node_data(_node_id)._size;
}

/**
 *
 */
void scenegraph::Node::
set_distance_relative(const bool v) {
    NodeData& nd = _get_node_data(_node_id);
    if (v != nd._distance_relative) {
        nd._distance_relative = v;
        nd._propagate_dirty(_sgdh);
    }
}

/**
 *
 */
bool scenegraph::Node::
get_distance_relative() {
    return _get_node_data(_node_id)._distance_relative;
}

/**
 *
 */
AABB scenegraph::Node::
get_aabb() {
    return _get_node_data(_node_id)._aabb;
}
