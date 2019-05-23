/**
 * Simple Quadtree implementation, partially based on information found on:
 * https://bit.ly/309V7J2
 */


#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include <vector>
#include "aabb.hpp"
#include "list_t.hpp"
#include "vector2.hpp"


namespace scenegraph {
    struct QuadNode {
        int first_child;
        int count;
    };

    struct QuadElement {
        int id;
        AABB aabb;
    };

    struct QuadElementNode {
        int next;
        int element;
    };

    class Quadtree {
    public:
        Quadtree();
        Quadtree(AABB& aabb, int max_leaf_elements, int max_depth);
        SmallList<int> query(AABB& aabb);
        bool insert(int id, AABB& aabb);
        bool move(int id, AABB& aabb_from, AABB& aabb_to);
        bool remove(int id, AABB& aabb);
        bool cleanup();
        bool inside(const double x, const double y);
        bool inside(Vector2& v);
        
    private:
        int _insert_element_node(AABB& aabb);
        void _leaf_to_branch(int node_id, AABB& aabb);

        AABB _aabb;
        FreeList<QuadElement> _elements;
        FreeList<QuadElementNode> _element_nodes;
        std::vector<QuadNode> _nodes;
        int _free_node;
        int _max_leaf_elements;
        int _max_depth;
        double _max_w, _max_h;
    };
}  // namespace scenegraph

#endif