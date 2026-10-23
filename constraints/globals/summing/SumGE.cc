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

    for(int posx : unassignedVariablesIdx) {
        Variable *x     = scope[posx];
        long      coeff = coefficients[posx];

        int sizeBefore = x->size();
        if(sizeBefore == 1)
            continue;
        if(coeff > 0) {
            long maxBase       = max - x->maximum() * coeff;
            long minimumBefore = x->minimum();

            if(maxBase + x->maximum() * coeff < limit)
                return false;
            for(int idv : x->domain) {
                long v = x->domain.toVal(idv);
                if(maxBase + v * coeff >= limit)
                    break;
                if(solver->delIdv(x, idv) == false)
                    return false;
            }
            if((min += (((long)x->minimum()) - ((long)minimumBefore)) * coeff) >= limit)
                return true;
        } else {
            long maxBase   = max - ((long)x->minimum()) * coeff;
            long maxBefore = x->maximum();
            if(maxBase + x->minimum() * coeff < limit)
                return false;

            for(int idv : reverse(x->domain)) {
                long v = x->domain.toVal(idv);
                if(maxBase + v * coeff >= limit)
                    break;
                if(maxBase + v * coeff < limit && solver->delIdv(x, idv) == false)
                    return false;
            }
            if(sizeBefore - x->size() > 0) {
                if((min += (((long)x->maximum()) - maxBefore) * coeff) >= limit)
                    return true;
            }
        }
        if(min >= limit)
            return solver->entail(this);
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
