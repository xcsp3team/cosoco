#ifndef SETSPARSE
#define SETSPARSE 1

#include <assert.h>

#include <iostream>

#include "mtl/Vec.h"
#include "mtl/VecIterator.h"
#include "utils/Verbose.h"

namespace Cosoco {

/**
 * SparseSet
 * can manage insertion, deletion... in O(1)
 * See Preston Briggs and Linda Torczon.
 *     An Efficient Representation for Sparse Sets”, 1993.
 *
 */
class SparseSet {
   protected:
    vec<int> positions;   // Correspond to sparse
    vec<int> elements;    // Correspond to dense
    int      limit;       // the number of elements
    int      capacity;    // The maximum number of elements
   public:
    using const_iterator         = VecIterator<const int>;
    using iterator               = VecIterator<int>;
    using const_reverse_iterator = VecIterator<const int>;
    using reverse_iterator       = VecIterator<int>;

    // Constructors
    SparseSet(int size, bool full = false) { setCapacity(size, full); }

    SparseSet(SparseSet const& copy) : limit(copy.limit), capacity(copy.capacity) {
        positions.growTo(copy.maxSize());
        elements.growTo(copy.maxSize());
        copy.positions.copyTo(positions);
        copy.elements.copyTo(elements);
    }

    SparseSet() { }

    void setCapacity(int cap, bool full) {
        capacity = cap;
        limit    = full ? cap : 0;
        positions.growTo(cap);
        elements.growTo(cap);
        for(int i = 0; i < cap; i++) {
            positions[i] = i;
            elements[i]  = i;
        }
    }


    // Fill and clear methods
    inline void clear() { limit = 0; }
    inline void fill() { limit = capacity; }


    // Questions about size methods
    inline bool      isEmpty() const { return (limit == 0); }
    inline int       size() const { return limit; }
    const inline int maxSize() const { return capacity; }


    // Insert and delete methods
    inline void add(const int k) {
        assert(k < maxSize());   // verify that we are bound safe
        if(contains(k))          // already there
            return;
        int tmp = elements[limit];
        int pos = positions[k];

        positions[tmp] = pos;
        elements[pos]  = tmp;

        elements[limit] = k;
        positions[k]    = limit;
        limit++;
    }

    inline void reduceTo(const int k) {
        clear();
        add(k);
    }

    inline void resetTo(SparseSet& set) {
        assert(capacity >= set.size());
        clear();
        for(int v : set) add(v);
    }

    inline int getPosition(const int k) {
        assert(k < size());
        return positions[k];
    }

    int shift() {
        assert(!isEmpty());
        int elt = elements[0];
        del(elements[0]);
        return elt;
    }


    inline void del(const int k) {
        assert(k < maxSize());
        if(!contains(k))
            return;
        limit--;
        int tmp         = elements[limit];
        int pos         = positions[k];
        positions[tmp]  = pos;
        elements[pos]   = tmp;
        elements[limit] = k;
        positions[k]    = limit;
    }

    int pop() {
        assert(limit >= 0);
        limit--;
        return elements[limit + 1];
    }

    // Access and contains method
    inline bool contains(const int k) const {
        assert(k <= maxSize());
        return positions[k] < limit;
    }
    inline int operator[](const int i) { return elements[i]; }

    // begin/end functions to use for each aka c++
    inline const_iterator cbegin() const { return const_iterator(elements.data); }
    inline const_iterator cend() const { return const_iterator(elements.data + limit); }
    inline iterator       begin() { return iterator(elements.data); }
    inline iterator       end() { return iterator(elements.data + limit); }


    inline const_reverse_iterator crbegin() const { return const_iterator(elements.data + limit - 1, -1); }
    inline const_reverse_iterator crend() const { return const_iterator(elements.data - 1, -1); }
    inline reverse_iterator       rbegin() { return iterator(elements.data + limit - 1, -1); }
    inline reverse_iterator       rend() { return iterator(elements.data - 1, -1); }


    // Display method
    void display() {
        printf("[");
        for(int i = 0; i < size(); i++) printf(" %d  ", elements[i]);
        printf("]");

        printf(" // [");
        for(int i = size(); i < maxSize(); i++) {
            printf("%s %d %s ", KRED, elements[i], KNRM);
        }
        printf("]");
    }
};
};   // namespace Cosoco

#endif