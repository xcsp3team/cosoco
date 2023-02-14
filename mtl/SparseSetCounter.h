//
// Created by audemard on 14/02/23.
//

#ifndef COSOCO_SPARSESETCOUNTER_H
#define COSOCO_SPARSESETCOUNTER_H
#include "SparseSet.h"

namespace Cosoco {
class SparseSetCounter : public SparseSet {
    vec<int> _counter;

   public:
    SparseSetCounter(int size) : SparseSet(size) { _counter.growTo(size, 0); }

    void clear() { SparseSet::clear(); }

    void add(const int k) {
        if(contains(k))
            _counter[k]++;
        else {
            SparseSet::add(k);
            _counter[k] = 1;
        }
    }

    int counter(int k) { return _counter[k]; }
};
}   // namespace Cosoco
#endif   // COSOCO_SPARSESETCOUNTER_H
