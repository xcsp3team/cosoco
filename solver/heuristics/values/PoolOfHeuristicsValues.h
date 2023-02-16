//
// Created by audemard on 16/02/23.
//

#ifndef COSOCO_POOLOFHEURISTICSVALUES_H
#define COSOCO_POOLOFHEURISTICSVALUES_H
#include "HeuristicVal.h"
#include "HeuristicValASGS.h"
#include "HeuristicValFirst.h"
#include "HeuristicValLast.h"
#include "HeuristicValOccs.h"
#include "HeuristicValRandom.h"


namespace Cosoco {
class PoolOfHeuristicsValues : public HeuristicVal {
   public:
    vec<HeuristicVal *> heuristicForVariable;
    HeuristicValASGS   *asgs;
    HeuristicValFirst  *first;
    HeuristicValLast   *last;
    HeuristicValOccs   *occs;
    HeuristicValRandom *random;

    explicit PoolOfHeuristicsValues(Solver &s);
    void selectHeuristics();

    int select(Variable *x) override;
};

}   // namespace Cosoco
#endif   // COSOCO_POOLOFHEURISTICSVALUES_H
