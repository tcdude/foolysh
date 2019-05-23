/**
 * Simple Quadtree implementation, partially based on information found on:
 * https://bit.ly/309V7J2
 */

#include "quadtree.hpp"

#include <algorithm>
#include <stdexcept>


/**
 * Default constructor, splits leafs at 8 elements and limits depth to 8.
 */
scenegraph::Quadtree::
Quadtree() {
    _aabb = AABB(0.0, 0.0, 1.0, 1.0);
    _max_leaf_elements = 8;
    _max_depth = 8;
    _max_w = 0.0;
    _max_h = 0.0;
    QuadNode root;
    root.first_child = -1;
    root.count = -1;
    _nodes.push_back(root);
}

/**
 * 
 */
scenegraph::Quadtree::
Quadtree(AABB& aabb, int max_leaf_elements, int max_depth) {
    _aabb = aabb;
    _max_leaf_elements = max_leaf_elements;
    _max_depth = max_depth;
    _max_w = 0.0;
    _max_h = 0.0;
    QuadNode root;
    root.first_child = -1;
    root.count = -1;
    _nodes.push_back(root);
}

/**
 * Return ``SmallList<int>`` with ids intersecting with ``aabb``.
 */
SmallList<int> scenegraph::Quadtree::
query(AABB& aabb) {
    SmallList<int> result;
    if (_nodes[0].first_child == -1) {
        return result;
    }
    SmallList<int> to_process;
    SmallList<AABB> quadrants;
    /* Extend search AABB to allow for cheap point inside AABB check */
    AABB search_aabb = AABB(aabb.x, aabb.y, aabb.hw + _max_w, aabb.hh + _max_h);
    
    to_process.push_back(0);
    quadrants.push_back(_aabb);
    while (to_process.size() > 0) {
        const int node_index = to_process.pop_back();
        AABB quadrant = quadrants.pop_back();
        if (_nodes[node_index].count == -1) {
            for (int i = 0; i < 4; ++i) {
                AABB search_quadrant = quadrant.split((Quadrant) i);
                if (search_aabb.overlap(search_quadrant)) {
                    to_process.push_back(_nodes[node_index].first_child + i);
                    quadrants.push_back(search_quadrant);
                }
            }
        }
        else {
            int element_node_id = _nodes[node_index].first_child;
            while (element_node_id != -1) {
                QuadElementNode& qen = _element_nodes[element_node_id];
                QuadElement& qe = _elements[qen.element];
                if (search_aabb.inside(qe.aabb.x, qe.aabb.y)) {
                    result.push_back(qe.id);
                }
                element_node_id = qen.next;
            }
        }
    }
    return result;
}

/**
 * Insert ``id`` into Quadtree using ``aabb``. Return ``true`` if successful.
 */
bool scenegraph::Quadtree::
insert(int id, AABB& aabb) {
    QuadElement qe;
    qe.id = id;
    qe.aabb = aabb;
    QuadElementNode qen;
    qen.element = _elements.insert(qe);
    qen.next = -1;
    int prev_qen_id = _insert_element_node(aabb);
    _element_nodes[prev_qen_id].next = _element_nodes.insert(qen);
    _max_w = std::max(_max_w, aabb.hw);
    _max_h = std::max(_max_h, aabb.hh);
    return true;
}

/**
 * Return last element_node index for insertion of an element and increments
 * the node count by 1.
 */
int scenegraph::Quadtree::
_insert_element_node(AABB& aabb) {
    AABB current_quadrant = _aabb;
    SmallList<int> to_process;
    int depth = 0;
    to_process.push_back(0);

    while (to_process.size() > 0) {
        const int node_index = to_process.pop_back();
        if (node_index == 0 && _nodes[node_index].first_child == -1) {
            _leaf_to_branch(node_index, current_quadrant);
        }
        else if (_nodes[node_index].count > -1){
            /* Leaf */
            if (_nodes[node_index].count < _max_leaf_elements || depth == _max_depth) {
                int element_node_id = _nodes[node_index].first_child;
                while (_element_nodes[element_node_id].next != -1) {
                    element_node_id = _element_nodes[element_node_id].next;
                }
                ++_nodes[node_index].count;
                return element_node_id;
            }
            else {
                _leaf_to_branch(node_index, current_quadrant);
            }
        }
        /* Is Branch */
        scenegraph::Quadrant quadrant = current_quadrant.find_quadrant(aabb.x, aabb.y);
        to_process.push_back(_nodes[node_index].first_child + (int) quadrant);
        current_quadrant = current_quadrant.split(quadrant);
        ++depth;
    }
    throw std::logic_error("Could not find appropriate element node!");
}

/**
 * Convert Leaf to Branch.
 */
void scenegraph::Quadtree::
_leaf_to_branch(int node_id, AABB& aabb) {
    SmallList<int> elements;
    if (_nodes[node_id].first_child != -1) {
        int element_node_id = _nodes[node_id].first_child;
        do {
            elements.push_back(_element_nodes[element_node_id].element);
            int next_id = _element_nodes[element_node_id].next;
            _element_nodes.erase(element_node_id);
            element_node_id = next_id;
        } while(element_node_id != -1);
    }
    int first_child = _nodes[node_id].first_child = _nodes.size();
    for (int i = 0; i < 4; ++i) {
        QuadNode qn;
        qn.first_child = -1;
        qn.count = 0;
        _nodes.push_back(qn);
    }
    _nodes[node_id].count = -1;
    while (elements.size() > 0) {
        int element_id = elements.pop_back();
        int child_node_id = first_child + (int) aabb.find_quadrant(
            _elements[element_id].aabb.x, _elements[element_id].aabb.y);
        QuadElementNode qen;
        qen.next = -1;
        qen.element = element_id;
        if (_nodes[child_node_id].first_child == -1) {
            _nodes[child_node_id].first_child = _element_nodes.insert(qen);
        }
        else {
            int element_node_id = _nodes[child_node_id].first_child;
            while (_element_nodes[element_node_id].next != -1) {
                element_node_id = _element_nodes[element_node_id].next;
            }
            _element_nodes[element_node_id].next = _element_nodes.insert(qen);
        }
        ++_nodes[child_node_id].count;
    }
}

/**
 * Move ``id`` in Quadtree from ``aabb_from`` to ``aabb_to``. Return ``true``
 * if successful.
 */
bool scenegraph::Quadtree::
move(int id, AABB& aabb_from, AABB& aabb_to) {
    if (remove(id, aabb_from)) {
        return insert(id, aabb_to);
    }
    return false;
}

/**
 * Remove ``id`` in Quadtree at ``aabb``. Return ``true`` if successful.
 */
bool scenegraph::Quadtree::
remove(int id, AABB& aabb) {
    if (_nodes[0].count != -1) {
        throw std::logic_error("Unable to remove, Quadtree is empty");
    }
    AABB current_quadrant = _aabb;
    SmallList<int> to_process;
    to_process.push_back(0);
    while (to_process.size() > 0) {
        const int node_index = to_process.pop_back();
        QuadNode& node = _nodes[node_index];
        if (node.count == -1) {
            Quadrant q = current_quadrant.find_quadrant(aabb.x, aabb.y);
            current_quadrant = current_quadrant.split(q);
            to_process.push_back(node.first_child + (int) q);
        }
        else {
            if (node.count == 0) {
                throw std::logic_error("Unable to remove, leaf is empty");
            }
            QuadElementNode& qen = _element_nodes[node.first_child];
            if (qen.next == -1) {
                if (_elements[qen.element].id != id) {
                    throw std::logic_error("Unable to remove, element not found");
                }
                _elements.erase(qen.element);
                _element_nodes.erase(node.first_child);
                node.first_child = -1;
                return true;
            }
            int last_element_node_id = node.first_child;
            while (qen.next != -1) {
                if (_elements[_element_nodes[qen.next].element].id == id) {
                    _elements.erase(_element_nodes[qen.next].element);
                    _element_nodes.erase(qen.next);
                    qen.next = _element_nodes[qen.next].next;
                    return true;
                }
                qen = _element_nodes[qen.next];
                last_element_node_id = qen.next;
            }
            throw std::logic_error("Unable to remove, element not found");
        }
    }
}

/**
 * Deferred cleanup of the Quadtree.
 */
bool scenegraph::Quadtree::
cleanup() {
    SmallList<int> to_process;
    if (_nodes[0].count == -1){
        to_process.push_back(0);
    }

    while (to_process.size() > 0) {
        const int node_index = to_process.pop_back();
        QuadNode& node = _nodes[node_index];

        int num_empty_leaves = 0;
        for (int j=0; j < 4; ++j) {
            const int child_index = node.first_child + j;
            const QuadNode& child = _nodes[child_index];

            if (child.count == 0) {
                ++num_empty_leaves;
            }
            else if (child.count == -1) {
                to_process.push_back(child_index);
            }
        }

        if (num_empty_leaves == 4) {
            _nodes[node.first_child].first_child = _free_node;
            _free_node = node.first_child;

            node.first_child = -1;
            node.count = 0;
        }
    }
    return true;
}

/**
 * Return true when point (``x``, ``y``) lie inside the quadtree.
 */
bool scenegraph::Quadtree::
inside(const double x, const double y) {
    return _aabb.inside(x, y);
}

/**
 * Return true when ``v`` lies inside the quadtree.
 */
bool scenegraph::Quadtree::
inside(Vector2& v) {
    return _aabb.inside(v[0], v[1]);
}
