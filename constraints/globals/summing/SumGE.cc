#include "SumGE.h"

#include "Sum.h"
#include "constraints/Constraint.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool SumWeightedGE::isSatisfiedBy(vec<int> &tuple) { return weightedSum(tuple) >= limit; }

bool SumSimpleGE::isSatisfiedBy(vec<int> &tuple) { return sum(tuple) >= limit; }

//----------------------------------------------
// Filtering
//----------------------------------------------

bool SumWeightedGE::filter(Variable *dummy) {
    computeBounds();
    if(min >= limit) {
        solver->entail(this);
        return true;
    }
    if(max < limit)
        return false;
    if(limit <= max - maxGap)
        return true;

    for(const int idx : unassignedVariablesIdx) {
        Variable *x = scope[idx];

        if(x->size() == 1)
            continue;
        long coeff = coefficients[idx];
        if(coeff >= 0) {
            min -= x->minimum() * coeff;
            if(delWRTOrder(x, limit - (max - x->maximum() * coeff), static_cast<int>(coeff), XCSP3Core::LT) == false)
                return false;
            min += x->minimum() * coeff;
        } else {
            min -= x->maximum() * coeff;
            if(delWRTOrder(x, limit - (max - x->minimum() * coeff), static_cast<int>(coeff), XCSP3Core::LT) == false)
                return false;
            min += x->maximum() * coeff;
        }
        if(min >= limit)
            return solver->entail(this);
        if(max < limit)
            return false;
    }


    return true;
}

bool SumSimpleGE::filter(Variable *dummy) {
    computeBounds();
    if(min >= limit)
        return solver->entail(this);

    if(max < limit)
        return false;
    bool useless = limit <= max - maxGap;
    if(useless == false)
        for(const int idx : unassignedVariablesIdx) {
            Variable *x = scope[idx];
            if(x->size() == 1)
                continue;
            min -= x->minimum();
            solver->delValuesLE(x, limit - (max - x->maximum()) - 1);
            min += x->minimum();
            if(min >= limit)
                return solver->entail(this);
        }
    return true;
}

//----------------------------------------------
// Objective constraint
//----------------------------------------------

void SumWeightedGE::updateBound(long bound) { limit = bound; }   // Update the current bound

void SumSimpleGE::updateBound(long bound) { limit = bound; }   // Update the current bound

long SumSimpleGE::maxUpperBound() {
    computeBounds();
    return max;
}

long SumWeightedGE::maxUpperBound() {
    computeBounds();
    return max;
}

long SumWeightedGE::minLowerBound() {
    computeBounds();
    return min;
}

long SumSimpleGE::minLowerBound() {
    computeBounds();
    return min;
}

long SumWeightedGE::computeScore(vec<int> &solution) { return weightedSum(solution); }

long SumSimpleGE::computeScore(vec<int> &solution) { return sum(solution); }

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------


SumWeightedGE::SumWeightedGE(Problem &p, vec<Variable *> &vars, vec<int> &coefs, long l) : WeightedSum(p, vars, coefs, l) {
    leftmostPositiveCoefficientPosition = coefficients.size();
    for(int i = coefficients.size() - 1; i >= 0; i--) {
        if(coefficients[i] < 0)
            break;
        leftmostPositiveCoefficientPosition = i;
    }
    type = "SumWeightedGE";
}
