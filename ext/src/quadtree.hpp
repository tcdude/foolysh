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
 * Simple Quadtree implementation, partially based on information found on:
 * https://bit.ly/309V7J2
 */


#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include <vector>
#include "aabb.hpp"
#include "list_t.hpp"
#include "vector2.hpp"

using tools::FreeList;
using tools::SmallList;

namespace tools {
    struct QuadNode {
        int first_child;
        int count;
    };

    struct QuadElement {
        int id;
        AABB aabb;
    };

    struct QuadElementNode {
        int next = -1;
        int element = -1;
    };

    class Quadtree {
    public:
        Quadtree();
        Quadtree(AABB& aabb, const int max_leaf_elements, const int max_depth);
        SmallList<int> query(AABB& aabb);
        bool insert(const int id, AABB& aabb);
        bool move(const int id, AABB& aabb_from, AABB& aabb_to);
        bool remove(const int id, AABB& aabb);
        bool cleanup();
        bool inside(const double x, const double y);
        bool inside(Vector2& v);
        void resize(AABB& aabb);

    private:
        int _insert_element_node(AABB& aabb);
        void _leaf_to_branch(const int node_id, AABB& aabb);

        AABB _aabb;
        FreeList<QuadElement> _elements;
        FreeList<QuadElementNode> _element_nodes;
        std::vector<QuadNode> _nodes;
        int _free_node;
        int _max_leaf_elements;
        int _max_depth;
        double _max_w, _max_h;
    };
}  // namespace tools

#endif
