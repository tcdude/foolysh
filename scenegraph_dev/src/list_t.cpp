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
SmallList<T>::SmallList() {
    _size = 0;
    _is_vec = false;
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
        _arr_vec.v.insert(_arr_vec.v.begin(), other._arr_vec.v.begin(), 
            other._arr_vec.v.end());
    }
    else {
        _size = other._size;
        _is_vec = false;
        std::copy(std::begin(other._arr_vec.a), std::end(other._arr_vec.a), 
            std::begin(_arr_vec.a));
    }
}

/**
 * 
 */
template <class T>
SmallList<T>::SmallList(SmallList<T>&& other) noexcept {
    if (other._is_vec) {
        _is_vec = true;
        _arr_vec.v = std::exchange(other._arr_vec.v, nullptr);
    }
    else {
        _size = other._size;
        _is_vec = false;
        _arr_vec.a = std::exchange(other._arr_vec.a, nullptr);
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
        std::swap(_arr_vec.v, other._arr_vec.v);
        _is_vec = true;
    }
    else {
        std::swap(_arr_vec.a, other._arr_vec.a);
        _is_vec = false;
        _size = other._size;
    }
    return *this;
}

/**
 * 
 */
template <class T>
void SmallList<T>::push_back(const T& element) {
    if (_is_vec) {
        _arr_vec.v.push_back(element);
    }
    else {
        if (_size < 128) {
            _arr_vec.a[_size] = element;
            ++_size;
        }
        else {
            _is_vec = true;
            _arr_vec.v.reset(new std::vector<T>);
            _arr_vec.v->reserve(256);
            for (auto it : _arr_vec.a) {
                _arr_vec.v->push_back(it);
            }
            _arr_vec.v->push_back(element);
        }
    }
}

/**
 * 
 */
template <class T>
T& SmallList<T>::pop_back() {
    if (_is_vec) {
        return _arr_vec.v->pop_back();
    }
    --_size;
    return _arr_vec.a[_size];
}

/**
 * 
 */
template <class T>
int SmallList<T>::size() {
    if (_is_vec) {
        return static_cast<int>(_arr_vec.v->size());
    }
    return _size;
}

/**
 * 
 */
template <class T>
T& SmallList<T>::operator[](int n) {
    if (_is_vec) {
        return *_arr_vec.v[n];
    }
    return _arr_vec.a[n];
}

/**
 * 
 */
template <class T>
const T& SmallList<T>::operator[](int n) const {
    if (_is_vec) {
        return *_arr_vec.v[n];
    }
    return _arr_vec.a[n];
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
        data[index].e.element = element;
        data[index].free = false;
        return index;
    }
    else {
        FreeElement fe;
        fe.e.element = element;
        data.push_back(fe);
        return static_cast<int>(data.size() - 1);
    }
}

/**
 * 
 */
template <class T>
void ExtFreeList<T>::erase(int n) {
    data[n].e.next = first_free;
    data[n].free = true;
    first_free = n;
    ++free_count;
    if (data.size() == free_count) {
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
bool ExtFreeList<T>::active( int n) {
    if (-1 < n < data.size()) {
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
