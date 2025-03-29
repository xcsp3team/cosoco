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
    vec<int>            unmatched;
    SparseSetMultiLevel unfixed;
    int                *predBFS;   // predBFS[idx] is the predecessor of variable x in the current BFS
    std::queue<int>     queueBFS;
    long               *visitTime;   // visitTime[n] is the time of the last visit (DFS) to node n (variable or value or T)
    long                time;        // For stamping


    int domainValue(int normalizedValue) const { return normalizedValue + minValue; }
    int normalizedValue(int v) const { return v - minValue; }

   public:
    explicit Matcher(Constraint *c);
    bool findMaximumMatching();
    bool findMatchingFor(int x);

    void notifyDeleteDecision(Variable *x, int v, Solver &s) override;
};
}   // namespace Cosoco

#endif   // COSOCO_MATCHER_H
