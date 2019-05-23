/**
 * Provides an indexed free list with constant-time removals from anywhere 
 * in the list without invalidating indices. T must be trivially constructible 
 * and destructible.
 */

#ifndef LIST_T_HPP
#define LIST_T_HPP

#include <memory>
#include <vector>


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
    T& pop_back();
    int size();
    T& operator[](int n);
    const T& operator[](int n) const;
private:
    union ArrVec {
        T a[128];
        std::unique_ptr<std::vector<T>> v;
        ~ArrVec() {delete[] a;}
    };
    ArrVec _arr_vec;
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
        union Val {
            T element;
            int next;
        } e;
        bool free = false;
    };
    std::vector<FreeElement> data;
    int first_free;
    int free_count;
};

#endif
