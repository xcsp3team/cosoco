//
// Created by audemard on 26/03/25.
//

#ifndef COSOCO_MATCHER_H
#define COSOCO_MATCHER_H

#include <ObserverDecision.h>
#include <SparseSetMultiLevel.h>

#include <queue>

#include "Constraint.h"

namespace Cosoco {
class Matcher : public ObserverDeleteDecision {
   protected:
    Constraint         *constraint;   // The associated constraint
    vec<Variable *>    &scope;
    int                 minValue, maxValue, interval;   // min, max and interval
    int                *val2var, *var2val;
    int                 arity;
    SparseSetMultiLevel unfixed;
    int                *pairU, *pairV, *dist;
    std::queue<int>     Q;

    int domainValue(int normalizedValue) const { return normalizedValue + minValue; }
    int normalizedValue(int v) const { return v - minValue; }

   public:
    explicit Matcher(Constraint *c);
    bool findMaximumMatching();
    void notifyDeleteDecision(Variable *x, int v, Solver &s) override;

   protected:
    bool bfs();
    bool dfs(int idx);
};
}   // namespace Cosoco

#endif   // COSOCO_MATCHER_H
