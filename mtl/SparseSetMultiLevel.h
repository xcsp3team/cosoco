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


    bool isLimitRecordedAtLevel(int level) { return level < limits.size() && limits[level] != NOT_STORED; }

    void recordLimit(int level) {
        if(level >= limits.size())
            limits.growTo(level + 1, NOT_STORED);
        assert(limits[level] == NOT_STORED);
        limits[level] = limit;
    }

   public:
    explicit SparseSetMultiLevel(int size, bool full = false) : SparseSet(size, full) { }


    SparseSetMultiLevel(SparseSetMultiLevel const &copy) : SparseSet(copy) { }


    SparseSetMultiLevel() { }


    void restoreLimit(int level) {
        if(isLimitRecordedAtLevel(level) == false)
            return;
        limit         = limits[level];
        limits[level] = NOT_STORED;
    }

    void del(const int k, int level) {
        if(isLimitRecordedAtLevel(level) == false)
            recordLimit(level);
        SparseSet::del(k);
    }

    void add(const int k, int level) {
        if(isLimitRecordedAtLevel(level) == false)
            recordLimit(level);
        SparseSet::add(k);
    }
};
};   // namespace Cosoco

#endif /* SPARSESETMULTILEVEL_H */
