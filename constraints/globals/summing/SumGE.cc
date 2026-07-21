#include "Sum.h"
#include "SumGE.h"
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
