#ifndef COSOCO_LINKEDSET_H
#define COSOCO_LINKEDSET_H

#include <iostream>
#include <ostream>

#include "Vec.h"

namespace Cosoco {

class LinkedSetIterator;

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


    int _lastRemoved;   // The last dropped element of the list.

    /**
     * The backward linking of all absent elements of the list (from last to first). An array index corresponds to an element. An
     * array value gives the previous absent element of the list or -1 if it does not exist. Hence, <code> prevsDel[i] == j
     * </code> means that j is the previously deleted element of the list before i.
     */
    vec<int> _prevRemoved;

    /**
     * The level at which absent elements have been removed from the list. An array index corresponds to an element. An array
     * value gives the level at which the corresponding element has been removed from the list. Hence, <code> absentLevels[i] == j
     * </code> means that j is the removal level of the element i and <code> absentLevels[i] == -1 </code> means that the element
     * i is present.
     */
    vec<int> removedLevels;

    int nbLevels;


    void del(const int a) {
        assert(a < maxSize());
        // if(!contains(a))
        //    return;

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
        _prevRemoved[a] = _lastRemoved;
        _lastRemoved    = a;
    }

    void add(const int a) {
        assert(_lastRemoved == a);
        // add to the list of present elements (works only if elements are managed as in a stack)
        int prev = prevs[a], next = nexts[a];
        if(prev == -1)
            _first = a;
        else
            nexts[prev] = a;
        if(next == -1)
            _last = a;
        else
            prevs[next] = a;
        // remove from the list of absent elements
        _lastRemoved = _prevRemoved[a];
    }

   public:
    // ---------- Create, initialize and fill
    LinkedSet(int sz, bool full) {
        assert(full);
        setCapacity(sz);
    }

    void setCapacity(int cap) {
        _size  = cap;
        _first = 0;
        _last  = cap - 1;
        prevs.growTo(cap);
        nexts.growTo(cap);
        _prevRemoved.growTo(cap, -1);
        removedLevels.growTo(cap, -1);
        fill();
    }

    void fill() {
        _first = 0;
        _last  = maxSize() - 1;
        for(int i = 0; i < prevs.size(); i++) prevs[i] = i - 1;
        for(int i = 0; i < nexts.size(); i++) nexts[i] = i + 1;
        nexts.last() = -1;
        _lastRemoved = -1;
        _prevRemoved.fill(-1);
        removedLevels.fill(-1);
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
        _size--;
        del(a);
    }


    void reduceTo(int a, int level) {
        if(isLimitRecordedAtLevel(level) == false)
            recordLimit(level);
        for(int b = _first; b != -1; b = next(b))
            if(b != a)
                del(b, level);
    }

    void delValuesGE(int maxId, int level) {
        display();
        std::cout << maxId << std::endl;
        for(int idv = last(); idv != -1; idv = prev(idv)) {   // Reverse traversal because of deletion
            if(idv < maxId)
                return;
            del(idv, level);
        }
        display();
    }

    void delValuesLE(int minId, int level) {
        for(int idv = first(); idv != -1; idv = next(idv)) {
            if(idv > minId)
                return;
            del(idv, level);
        }
    }
    // ---------- Access and contains method and iterators
    bool contains(const int k) const {
        assert(k < maxSize());
        return removedLevels[k] == -1;
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

    int lastRemoved() const { return _lastRemoved; }
    int prevRemoved(int id) { return _prevRemoved[id]; }


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

    int lastRemovedLevel() { return _lastRemoved == -1 ? -1 : removedLevels[_lastRemoved]; }

    void restoreLastDropped() {
        assert(_lastRemoved != -1 && !contains(_lastRemoved));
        removedLevels[_lastRemoved] = -1;
        _size++;
        add(_lastRemoved);
    }

    void restoreLimit(int level) {
        assert(_lastRemoved == -1 || removedLevels[_lastRemoved] <= level);
        for(int a = _lastRemoved; a != -1 && removedLevels[a] >= level; a = _lastRemoved) restoreLastDropped();
        limits[level] = NOT_STORED;
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
