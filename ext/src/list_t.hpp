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
 * Provides an indexed free list with constant-time removals from anywhere
 * in the list without invalidating indices. T must be trivially constructible
 * and destructible.
 */

#ifndef LIST_T_HPP
#define LIST_T_HPP

#include <memory>
#include <vector>

namespace tools {
    template <class T>
    class FreeList {
    public:
        FreeList();
        int insert(const T& element);
        void erase(int n);
        void clear();
        int range() const;
        T& operator[](int n);
        const T& operator[](int n) const;

    private:
        union FreeElement {
            FreeElement() {}
            ~FreeElement() {}
            T element;
            int next;
        };
        std::vector<FreeElement> data;
        int first_free;
    };


    template <class T>
    class SmallList {
    public:
        SmallList();
        ~SmallList();
        SmallList(const SmallList<T>& other);
        SmallList(SmallList<T>&& other) noexcept;
        SmallList<T>& operator=(const SmallList<T>& other);
        SmallList<T>& operator=(SmallList<T>&& other) noexcept;
        void push_back(const T& element);
        T pop_back();
        int size();
        T& operator[](int n);
        const T& operator[](int n) const;
    private:
        T _a[128];
        std::vector<T> _v;
        int _size;
        bool _is_vec;
    };


    template <class T>
    class ExtFreeList {
    public:
        ExtFreeList();
        int insert(const T& element);
        void erase(int n);
        void clear();
        int range() const;
        bool active(int n);
        T& operator[](int n);
        const T& operator[](int n) const;

    private:
        struct FreeElement {
            T element;
            int next;
            bool free = false;
        };
        std::vector<FreeElement> data;
        int first_free;
        int free_count;
    };
}  // namespace tools

#endif
