//
// Created by audemard on 20/07/2026.
//

#ifndef COSOCO_LINKEDSETOPT_H
#define COSOCO_LINKEDSETOPT_H
#include "LinkedSet.h"

namespace Cosoco {
/*
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

class LinkedSetOpt : public LinkedSet {
   protected:
    vec<Slice> stackedSlices;
    vec<int>   removedStack;

    bool insideBounds(int idv) const { return _first <= idv && idv <= _last; }
    bool isConnex() { return _last - _first + 1 == _size; }

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

   public:
    LinkedSetOpt(int sz, bool full) : LinkedSet(sz, full) { }

    bool contains(const int idv) const override { return insideBounds(idv) && LinkedSet::contains(idv); }

    void reduceTo(int a, int level) override {
        assert(contains(a));
        delValuesGE(a + 1, level);
        delValuesLE(a - 1, level);
    }

    void delValuesLE(int minId, int level) override {
        if(minId == _first)
            LinkedSet::del(minId, level);
        assert(_first < minId && minId < _last);

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
        removedStack.push(-1);
    }

    void delValuesGE(int maxId, int level) override {
        if(maxId == _last)
            return LinkedSet::del(maxId, level);
        assert(_first < maxId && maxId < _last);

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
        removedStack.push(-1);
    }

   protected:
    void del(const int a) override {
        LinkedSet::del(a);
        removedStack.push(a);
    }

   public:
    void restoreLastDropped() override {
        assert(false);
        // TODO
    }

    void restoreLimit(int level) override {
        while(removedStack.size() > 0) {
            int k = removedStack.last();
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
            } else {   // stand-alone removed value
                // TODO assert control k is lastRemoved
                if(removedLevels[k] < level)
                    break;
                removedLevels[k] = -1;
                _size++;
                add(k);
            }
            removedStack.pop();
        }
    }

    int lastRemovedLevel() override {
        if(removedStack.size() == -1)
            return -1;
        int k = removedStack.last();
        if(k == -1)
            return stackedSlices.last().level;
        return removedLevels[k];
    }


    int prevRemoved(int id) override {
        assert(false);   // TODO
    }

    int lastRemoved() const override {
        if(removedStack.size() == 0)
            return -1;
        int k = removedStack.last();
        if(k < 0)
            return stackedSlices.last().bound;
        return k;
    }
};
*/
}   // namespace Cosoco
#endif   // COSOCO_LINKEDSETOPT_H
