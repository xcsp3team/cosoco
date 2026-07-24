#ifndef COSOCO_LINKEDSET_H
#define COSOCO_LINKEDSET_H

#include <iostream>

#include "Vec.h"

namespace Cosoco {

class LinkedSetIterator;

struct Slice {
    Slice(bool left_side, int idx, int a, int bound, int size, int level)
        : leftSide(left_side), idx(idx), a(a), bound(bound), size(size), level(level) { }
    bool leftSide;
    int  idx;
    int  a;
    int  bound;
    int  size;
    int  level;
};


class LinkedSet {
   protected:
    int _size;    // The number of elements present
    int _first;   // The first (present) element of the list.
    int _last;    // The last (present) element of the list.

    // Useless but limits the modification in the code, can be usefull if we want to deal with
    // Different data structures for domains
    vec<int>  limits;
    const int NOT_STORED = -1;


    /**
     * The backward linking of all present elements of the list (from last to first). An array index corresponds to an element. An
     * array value gives the previous present element of the list or -1 if it does not exist. Hence, <code> prevs[i] == j </code>
     * means that j is the previous present element in the list before i.
     */
    vec<int> prevs;

    /**
     * The forward linking of all present elements of the list (from first to last). An array index corresponds to an element. An
     * array value gives the next present element of the list or -1 if it does not exist. Hence, <code> nexts[i] == j </code>
     * means that j is the next present element in the list after i.
     */
    vec<int> nexts;


    /**
     * The stack of removed elements
     */
    vec<int> removed;
    /**
     * The level at which absent elements have been removed from the list. An array index corresponds to an element. An array
     * value gives the level at which the corresponding element has been removed from the list. Hence, <code> absentLevels[i] == j
     * </code> means that j is the removal level of the element i and <code> absentLevels[i] == -1 </code> means that the element
     * i is present.
     */
    vec<int> removedLevels;
    // ------------------------------------------------------------------------------
    // Structures for the optimized version. No use of subclasses because of cost :(
    // ------------------------------------------------------------------------------

    bool       isOptimized;
    vec<Slice> stackedSlices;

    bool insideBounds(int idv) const { return _first <= idv && idv <= _last; }
    bool isConnex() const { return _last - _first + 1 == _size; }

    int newSizeUpto(int a) {
        assert(_first <= a && contains(a));
        if(isConnex())   // no holes
            return _size - (a - _first + 1);
        // possibly, we can reason from stand-alone removed values if put in a list to compute newSize
        int cnt = 0;
        for(int b = _first; b != a; b = nexts[b]) cnt++;
        return _size - (cnt + 1);
    }

    int newSizeFrom(int idv) {
        assert(idv <= _last && contains(idv));
        if(isConnex())   // no holes
            return _size - (_last - idv + 1);
        // possibly, a) we can reason from stand-alone removed values if put in a list to compute newSize
        // b) one can iterate over the domain at the right or the left of a
        int cnt = 0;
        for(int b = idv; b != -1; b = nexts[b]) cnt++;
        return _size - cnt;
    }

    void del(const int a) {
        assert(a < maxSize());

        // remove from the list of present elements
        int prev = prevs[a], next = nexts[a];
        if(prev == -1)
            _first = next;
        else
            nexts[prev] = next;
        if(next == -1)
            _last = prev;
        else
            prevs[next] = prev;
        // add to the end of the list of absent elements
        removed.push(a);
        _size--;
    }

    void add() {
        // add to the list the last element removed
        int a    = removed.last();
        int prev = prevs[a], next = nexts[a];
        if(prev == -1)
            _first = a;
        else
            nexts[prev] = a;
        if(next == -1)
            _last = a;
        else
            prevs[next] = a;
        removed.pop();
        _size++;
    }

   public:
    // ---------- Create, initialize and fill
    LinkedSet(int sz, bool full, bool isoptimized = false) : isOptimized(isoptimized) {
        assert(full);
        setCapacity(sz);
    }


    void setCapacity(int cap) {
        _size  = cap;
        _first = 0;
        _last  = cap - 1;
        prevs.growTo(cap);
        nexts.growTo(cap);
        removedLevels.growTo(cap, -1);
        removed.growTo(cap);
        fill();
    }

    void fill() {
        _first = 0;
        _last  = maxSize() - 1;
        for(int i = 0; i < prevs.size(); i++) prevs[i] = i - 1;
        for(int i = 0; i < nexts.size(); i++) nexts[i] = i + 1;
        nexts.last() = -1;
        removedLevels.fill(-1);
        removed.clear();
        stackedSlices.clear();
        _size = maxSize();
    }


    // ----------  Questions about size methods

    bool isEmpty() const { return _size == 0; }
    int  size() const { return _size; }
    int  maxSize() const { return prevs.size(); }


    // ---------- Del methods
    void del(int a, int level) {   // Return true if level is now recorded
        if(isLimitRecordedAtLevel(level) == false)
            recordLimit(level);
        removedLevels[a] = level;
        del(a);
    }


    void reduceTo(int a, int level) {
        if(isOptimized) {
            if(a + 1 <= _last)
                delValuesGE(a + 1, level);
            if(a - 1 >= _first)
                delValuesLE(a - 1, level);
            assert(_size == 1);
            return;
        }
        if(isLimitRecordedAtLevel(level) == false)
            recordLimit(level);
        for(int b = _first; b != -1; b = next(b))
            if(b != a)
                del(b, level);
    }

    void delValuesGE(int maxId, int level) {
        if(maxId == _last)
            return del(maxId, level);
        assert(_first <= maxId && maxId < _last);
        // std::cout << maxId << " " << _last << std::endl;

        if(isLimitRecordedAtLevel(level) == false)
            recordLimit(level);
        if(contains(maxId) == false)
            maxId = next(maxId);
        int curSize = size(), newSize = newSizeFrom(maxId);


        int p    = prevs[maxId];
        nexts[p] = -1;   // was a
        int bnd  = _last;
        _last    = p;
        _size    = newSize;
        stackedSlices.push({false, p, maxId, bnd, curSize, level});
        removed.push(-1);
    }

    void delValuesLE(int minId, int level) {
        if(minId == _first)
            return del(minId, level);
        // std::cout << minId << " " << _first << std::endl;

        assert(_first < minId && minId <= _last);

        if(isLimitRecordedAtLevel(level) == false)
            recordLimit(level);

        if(contains(minId) == false)
            minId = prev(minId);

        int curSize = size(), newSize = newSizeUpto(minId);


        int n    = nexts[minId];
        prevs[n] = -1;   // was a
        int bnd  = _first;
        _first   = n;
        _size    = newSize;

        stackedSlices.push({true, n, minId, bnd, curSize, level});
        removed.push(-1);
    }

    // ---------- Access and contains method and iterators
    bool contains(const int k) const {
        assert(k < maxSize());
        return removedLevels[k] == -1 && insideBounds(k);
    }

    int operator[](const int i) {
        assert(0 <= i && i < size());
        int e = first();
        for(int cnt = 0; cnt < i; cnt++) e = next(e);
        return e;
    }

    using iterator         = LinkedSetIterator;
    using reverse_iterator = LinkedSetIterator;

    iterator         begin();
    iterator         end();
    reverse_iterator rbegin();
    reverse_iterator rend();

    int lastRemoved() const {
        if(removed.size() == 0)
            return -1;
        if(isOptimized) {
            int k = removed.last();
            if(k < 0)
                return stackedSlices.last().bound;
            return k;
        }
        return removed.last();
    }
    int prevRemoved(int id) {
        assert(false);
        //    return _prevRemoved[id];
    }


    int first() { return _first; }
    int last() { return _last; }


    int next(int a) {
        assert(a >= 0 && a < maxSize());
        if(removedLevels[a] == -1)
            return nexts[a];
        // below a can not be equal to first since a is not present
        if(a < _first)
            return _first;
        int next = nexts[a];
        if(next == -1 || next > _last)
            return -1;
        while(removedLevels[next] != -1) next = nexts[next];
        return next;
    }


    int prev(int a) {
        assert(a >= 0 && a < maxSize());
        if(removedLevels[a] == -1)
            return prevs[a];
        // below a can not be equal to last since a is not present
        if(a > _last)
            return _last;
        int prev = prevs[a];
        if(prev < _first)   // includes prev == -1
            return -1;
        while(removedLevels[prev] != -1) prev = prevs[prev];
        return prev;
    }


    int lastRemovedLevel() {
        if(removed.size() == -1)
            return -1;
        int k = removed.last();
        if(k == -1)
            return stackedSlices.last().level;
        return removedLevels[k];
    }


    void restoreLimit(int level) {
        while(removed.size() > 0) {
            int k = removed.last();
            if(k == -1) {   // meaning a slice
                assert(stackedSlices.size() > 0);
                Slice &slice = stackedSlices.last();
                if(slice.level < level)
                    break;
                if(slice.leftSide) {
                    prevs[slice.idx] = slice.a;
                    _first           = slice.bound;
                } else {
                    nexts[slice.idx] = slice.a;
                    _last            = slice.bound;
                }
                _size = slice.size;
                stackedSlices.pop();
                removed.pop();
            } else {   // stand-alone removed value
                if(removedLevels[k] < level)
                    break;
                removedLevels[k] = -1;
                add();   //  removed.pop() is done inside add
            }
        }
    }
    // Display method
    void display() {
        printf("[");
        for(int i = first(); i != -1; i = next(i)) printf(" %d  ", i);
        printf("]");

        printf(" // [");
        for(int i = 0; i < maxSize(); i++)
            if(contains(i) == false)
                printf(" %d  ", i);

        printf("]");
        printf("\nprevs: ");
        for(int prev : prevs) printf("%d ", prev);
        printf("\nnext: ");
        for(int i = 0; i < nexts.size(); i++) printf("%d ", nexts[i]);
        printf("\nremoved");
        for(int i = 0; i < removedLevels.size(); i++) printf("%d ", removedLevels[i]);
        printf("\n");
    }

   protected:
    // Related to levels.
    bool isLimitRecordedAtLevel(int level) { return level < limits.size() && limits[level] != NOT_STORED; }

    void recordLimit(int level) {
        if(level >= limits.size())
            limits.growTo(level + 1, NOT_STORED);
        assert(limits[level] == NOT_STORED);
        limits[level] = _size;
    }
};
}   // namespace Cosoco
#endif   // COSOCO_LINKEDSET_H
