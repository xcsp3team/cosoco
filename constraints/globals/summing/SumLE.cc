//
// Created by audemard on 22/07/2026.
//

#include "SumLE.h"

#include "Solver.h"

using namespace Cosoco;
//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

// bool WeightedSumGE::isSatisfiedBy(vec<int> &tuple) { return weightedSum(tuple) >= limit; }

bool SumSimpleLE::isSatisfiedBy(vec<int> &tuple) { return sum(tuple) <= limit; }

//----------------------------------------------
// Filtering
//----------------------------------------------

bool SumSimpleLE::filter(Variable *dummy) {
    computeBounds();
    if(max <= limit)
        return solver->entail(this);

    if(min > limit)
        return false;

    bool useless = min + maxGap <= limit;
    if(useless == false)
        for(const int idx : unassignedVariablesIdx) {
            Variable *x = scope[idx];
            if(x->size() == 1)
                continue;
            max -= x->maximum();
            solver->delValuesGE(x, limit - (min - x->minimum()) + 1);
            max += x->maximum();
            if(max <= limit)
                return solver->entail(this);
        }
    return true;
}
//----------------------------------------------
// Objective constraint
//----------------------------------------------


void SumSimpleLE::updateBound(long bound) { limit = bound; }   // Update the current bound


long SumSimpleLE::maxUpperBound() {
    computeBounds();
    return max;
}

long SumSimpleLE::minLowerBound() {
    computeBounds();
    return min;
}


long SumSimpleLE::computeScore(vec<int> &solution) { return sum(solution); }

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------
