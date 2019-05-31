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
 */

/**
 * Provides an indexed free list with constant-time removals from anywhere
 * in the list without invalidating indices. T must be trivially constructible
 * and destructible.
 */

#include "list_t.hpp"
#include <algorithm>
#include <iterator>

/**
 * FreeList
 */

/**
 *
 */
template <class T>
FreeList<T>::FreeList(): first_free(-1) {}

/**
 *
 */
template <class T>
int FreeList<T>::insert(const T& element) {
    if (first_free != -1) {
        const int index = first_free;
        first_free = data[first_free].next;
        data[index].element = element;
        return index;
    }
    else {
        FreeElement fe;
        fe.element = element;
        data.push_back(fe);
        return static_cast<int>(data.size() - 1);
    }
}

/**
 *
 */
template <class T>
void FreeList<T>::erase(int n) {
    data[n].next = first_free;
    first_free = n;
}

/**
 *
 */
template <class T>
void FreeList<T>::clear() {
    data.clear();
    first_free = -1;
}

/**
 *
 */
template <class T>
int FreeList<T>::range() const {
    return static_cast<int>(data.size());
}

/**
 *
 */
template <class T>
T& FreeList<T>::operator[](int n) {
    return data[n].element;
}

/**
 *
 */
template <class T>
const T& FreeList<T>::operator[](int n) const {
    return data[n].element;
}


/**
 * SmallList
 */

/**
 *
 */
template <class T>
SmallList<T>::SmallList() : _size(0), _is_vec(false) {
}

/**
 *
 */
template <class T>
SmallList<T>::~SmallList() {
}

/**
 *
 */
template <class T>
SmallList<T>::SmallList(const SmallList<T>& other) {
    if (other._is_vec) {
        _is_vec = true;
        _v.insert(_v.begin(), other._v.begin(), other._v.end());
    }
    else {
        _size = other._size;
        _is_vec = false;
        std::copy(std::begin(other._a), std::end(other._a), std::begin(_a));
    }
}

/**
 *
 */
template <class T>
SmallList<T>::SmallList(SmallList<T>&& other) noexcept {
    if (other._is_vec) {
        _is_vec = true;
        _v.swap(other._v);
    }
    else {
        _size = other._size;
        _is_vec = false;
        std::swap(_a, other._a);
    }
}

/**
 *
 */
template <class T>
SmallList<T>& SmallList<T>::operator=(const SmallList<T>& other) {
    return *this = SmallList(other);
}

/**
 *
 */
template <class T>
SmallList<T>& SmallList<T>::operator=(SmallList<T>&& other) noexcept {
    if (other._is_vec) {
        _is_vec = true;
        _v.swap(other._v);
    }
    else {
        _size = other._size;
        _is_vec = false;
        std::swap(_a, other._a);
    }
    return *this;
}

/**
 *
 */
template <class T>
void SmallList<T>::push_back(const T& element) {
    if (_is_vec) {
        _v.push_back(element);
    }
    else {
        if (_size < 128) {
            _a[_size] = element;
            ++_size;
        }
        else {
            _is_vec = true;
            _v.reserve(256);
            for (auto it : _a) {
                _v.push_back(it);
            }
            _v.push_back(element);
        }
    }
}

/**
 *
 */
template <class T>
T SmallList<T>::pop_back() {
    if (_is_vec) {
        T ret = _v.back();
        _v.pop_back();
        return ret;
    }
    --_size;
    return _a[_size];
}

/**
 *
 */
template <class T>
int SmallList<T>::size() {
    if (_is_vec) {
        return static_cast<int>(_v.size());
    }
    return _size;
}

/**
 *
 */
template <class T>
T& SmallList<T>::operator[](int n) {
    if (_is_vec) {
        return _v[n];
    }
    return _a[n];
}

/**
 *
 */
template <class T>
const T& SmallList<T>::operator[](int n) const {
    if (_is_vec) {
        return _v[n];
    }
    return _a[n];
}


/**
 * ExtFreeList
 */

/**
 *
 */
template <class T>
ExtFreeList<T>::ExtFreeList(): first_free(-1), free_count(0) {}

/**
 *
 */
template <class T>
int ExtFreeList<T>::insert(const T& element) {
    if (first_free != -1) {
        const int index = first_free;
        first_free = data[first_free].next;
        data[index].element = element;
        data[index].free = false;
        return index;
    }
    else {
        //FreeElement fe();
        //fe.e.element = element;
        data.push_back(FreeElement());
        data.back().element = element;
        return static_cast<int>(data.size() - 1);
    }
}

/**
 *
 */
template <class T>
void ExtFreeList<T>::erase(int n) {
    data[n].next = first_free;
    data[n].free = true;
    first_free = n;
    ++free_count;
    if ((int) data.size() == free_count) {
        clear();
    }
}

/**
 *
 */
template <class T>
void ExtFreeList<T>::clear() {
    data.clear();
    first_free = -1;
    free_count = 0;
}

/**
 *
 */
template <class T>
int ExtFreeList<T>::range() const {
    return static_cast<int>(data.size());
}

/**
 *
 */
template <class T>
bool ExtFreeList<T>::active(int n) {
    if (n < (int) data.size() && n > -1) {
        return (data[n].free) ? false : true;
    }
    return false;
}

/**
 *
 */
template <class T>
T& ExtFreeList<T>::operator[](int n) {
    return data[n].element;
}

/**
 *
 */
template <class T>
const T& ExtFreeList<T>::operator[](int n) const {
    return data[n].element;
}
