//
// Created by audemard on 08/02/2022.
//

#ifndef COSOCO_BINPACKING_H
#define COSOCO_BINPACKING_H

#include "ObserverDecision.h"
#include "constraints/globals/GlobalConstraint.h"
#include "mtl/SparseSetMultiLevel.h"

namespace Cosoco {

class Bin {
   public:
    int index;
    int capacity;   // the capacity is updated when possible (when an object is guaranteed to be in it)
    int lost;       // only used when reasoning energetically
    int minSizeObj;
    int maxSizeObj;

    void set(int i, int c) {
        index    = i;
        capacity = c;
        lost     = 0;
    }
};


struct CompareBins {
    bool operator()(Bin *b1, Bin *b2) { return b1->capacity < b2->capacity; }
};


class BinPacking : public GlobalConstraint, ObserverDeleteDecision {
   protected:
    int      nItems;   // The number of items to be put in the bins
    int      nBins;    //  The number of bins
    vec<int> sizes;    // sizes[i] is the size of the ith item
    vec<int> limits;   // limits[j] is the capacity of the jth bin

    // Propagators data structures
    vec<int>   sums;         // Temporary array
    vec<Bin *> bins;         // The possible bins
    vec<Bin *> sortedBins;   // The array of bins, sorted according to their current remaining capacities

    /**
     * fronts[i] is the set of items which are in front of the ith bin (in the ordered sequence of bins) such that i
     * is the first position where they can be put
     */
    vec<SparseSet>      fronts;
    SparseSetMultiLevel usableBins;

   public:
    BinPacking(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &_sizes, vec<int> &_limits);
    bool isCorrectlyDefined() override;
    void delayedConstruction(int id) override;
    void attachSolver(Solver *s) override;
    void notifyDeleteDecision(Variable *x, int v, Solver &s) override;


    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco
#endif   // COSOCO_BINPACKING_H
