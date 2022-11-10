#ifndef SPARSESETMULTILEVEL_H
#define SPARSESETMULTILEVEL_H

#include "SparseSet.h"
#include "mtl/Vec.h"

namespace Cosoco {

class SparseSetMultiLevel : public SparseSet {
    friend class Solver;

   protected:
    vec<int>  limits;
    const int NOT_STORED = -1;

   public:
    SparseSetMultiLevel(int size, bool full = false) : SparseSet(size, full) { }


    SparseSetMultiLevel(SparseSetMultiLevel const &copy) : SparseSet(copy) { }


    SparseSetMultiLevel() { }


    bool isLimitRecordedAtLevel(int level) { return level < limits.size() && limits[level] != NOT_STORED; }


    void recordLimit(int level) {
        if(level >= limits.size())
            limits.growTo(level + 1, NOT_STORED);
        assert(limits[level] == NOT_STORED);
        limits[level] = limit;
    }


    void restoreLimit(int level) {
        assert(limits[level] != NOT_STORED);
        limit         = limits[level];
        limits[level] = NOT_STORED;
    }
};
};   // namespace Cosoco

#endif /* SPARSESETMULTILEVEL_H */
