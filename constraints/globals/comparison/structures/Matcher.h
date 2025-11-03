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
    int                 minValue, maxValue, interval, nNodes, T;   // min, max and interval
    int                *var2val;
    int                 arity;
    vec<int>            unmatched;
    SparseSetMultiLevel unfixed;
    int                *predBFS;     // predBFS[idx] is the predecessor of variable x in the current BFS
    long               *visitTime;   // visitTime[n] is the time of the last visit (DFS) to node n (variable or value or T)
    long                time;        // For stamping


    // Data Structure for Tarjan algorithm
    SparseSet           stackTarjan;
    bool                splitSCC;
    int                 nVisitedNodes;
    SparseSetMultiLevel fixedVars;
    vec<SparseSet>      neighborsOfValues;
    SparseSet           neighborsOfT, varsOutSCC, valsInSCC;
    int                *numDFS;
    int                *lowLink;

    int domainValue(int normalizedValue) const { return normalizedValue + minValue; }
    int normalizedValue(int v) const { return v - minValue; }

    virtual void computeNeighbors() = 0;

   public:
    explicit Matcher(Constraint *c);
    virtual bool findMaximumMatching() = 0;

    void notifyDeleteDecision(Variable *x, int v, Solver &s) override;
    void removeInconsistentValues();
    void tarjanRemoveValues(int node);
};

class MatcherAllDifferent : public Matcher {
    int *val2var;

   public:
    explicit MatcherAllDifferent(Constraint *c);
    bool findMaximumMatching() override;
    void computeNeighbors() override;
    bool findMatchingFor(int x);
};

class MatcherCardinality : public Matcher {
    vec<SparseSet> val2var;
    vec<int>       keys;
    int           *minOccs, *maxOccs;
    int           *predValue;
    vec<SparseSet> possibleVars;
    SparseSet      queueBFS;


    void handleAugmentingPath(int x, int v);
    bool findMatchingForValue(int u);
    bool findMatchingForVariable(int x);

   public:
    explicit MatcherCardinality(Constraint *c, vec<int> &_keys, vec<int> &_minOccs, vec<int> &_maxOccs);
    bool findMaximumMatching() override;
    void computeNeighbors() override;
    bool findMatchingFor(int x);
};
}   // namespace Cosoco

#endif   // COSOCO_MATCHER_H
