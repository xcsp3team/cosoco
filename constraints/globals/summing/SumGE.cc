#include "SumGE.h"

#include "Sum.h"
#include "constraints/Constraint.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool WeightedSumGE::isSatisfiedBy(vec<int> &tuple) { return weightedSum(tuple) >= limit; }

bool SumGE::isSatisfiedBy(vec<int> &tuple) { return sum(tuple) >= limit; }
//----------------------------------------------
// Filtering
//----------------------------------------------

bool WeightedSumGE::filter(Variable *dummy) {
    computeBounds();
    if(min >= limit) {
        solver->entail(this);
        return true;
    }
    if(max < limit)
        return false;

    bool useless = limit <= max - maxGap;
    if(useless == false)
        for(const int idx : unassignedVariablesIdx) {
            Variable *x = scope[idx];

            if(x->size() == 1)
                continue;
            long coeff = coefficients[idx];
            if(coeff >= 0) {
                min -= x->minimum() * coeff;
                delWRTOrder(x, limit - (max - x->maximum() * coeff), static_cast<int>(coeff), XCSP3Core::LT);
                min += x->minimum() * coeff;
            } else {
                min -= x->maximum() * coeff;
                delWRTOrder(x, limit - (max - x->minimum() * coeff), static_cast<int>(coeff), XCSP3Core::LT);
                min += x->maximum() * coeff;
            }
            if(min >= limit)
                return solver->entail(this);
        }
    return true;
}

bool SumGE::filter(Variable *dummy) {
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

void WeightedSumGE::updateBound(long bound) { limit = bound; }   // Update the current bound

void SumGE::updateBound(long bound) { limit = bound; }   // Update the current bound

long WeightedSumGE::maxUpperBound() {
    computeBounds();
    return max;
}

long SumGE::maxUpperBound() {
    computeBounds();
    return max;
}

long SumGE::minLowerBound() {
    computeBounds();
    return min;
}

long WeightedSumGE::minLowerBound() {
    computeBounds();
    return min;
}

long WeightedSumGE::computeScore(vec<int> &solution) { return weightedSum(solution); }

long SumGE::computeScore(vec<int> &solution) { return sum(solution); }

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------


WeightedSumGE::WeightedSumGE(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &coefs, long l)
    : WeightedSum(p, n, vars, coefs, l) {
    leftmostPositiveCoefficientPosition = coefficients.size();
    for(int i = coefficients.size() - 1; i >= 0; i--) {
        if(coefficients[i] < 0)
            break;
        leftmostPositiveCoefficientPosition = i;
    }
}
