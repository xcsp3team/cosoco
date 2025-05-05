#include "SumGE.h"

#include "Sum.h"
#include "constraints/Constraint.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool SumGE::isSatisfiedBy(vec<int> &tuple) { return weightedSum(tuple) >= limit; }

//----------------------------------------------
// Filtering
//----------------------------------------------

bool SumGE::filter(Variable *dummy) {
    computeBounds();
    if(min >= limit) {
        solver->entail(this);
        return true;
    }
    if(max < limit)
        return false;

    for(int posx : unassignedVariablesIdx) {
        Variable *x     = scope[posx];
        int       coeff = coefficients[posx];

        int sizeBefore = x->size();
        if(sizeBefore == 1)
            continue;
        if(coeff > 0) {
            long maxBase       = max - x->maximum() * coeff;
            int  minimumBefore = x->minimum();

            if(maxBase + x->maximum() * coeff < limit)
                return false;
            for(int idv : x->domain) {
                int v = x->domain.toVal(idv);
                if(maxBase + v * coeff >= limit)
                    break;
                if(solver->delIdv(x, idv) == false)
                    return false;
            }
            if((min += (x->minimum() - minimumBefore) * coeff) >= limit)
                return true;
        } else {
            long maxBase   = max - x->minimum() * coeff;
            int  maxBefore = x->maximum();
            if(maxBase + x->minimum() * coeff < limit)
                return false;

            for(int idv : reverse(x->domain)) {
                int v = x->domain.toVal(idv);
                if(maxBase + v * coeff >= limit)
                    break;
                if(maxBase + v * coeff < limit && solver->delIdv(x, idv) == false)
                    return false;
            }
            if(sizeBefore - x->size() > 0) {
                if((min += (x->maximum() - maxBefore) * coeff) >= limit)
                    return true;
            }
        }
    }
    return true;
}


void SumGE::computeBounds() {
    min = max = 0;
    for(int i = 0; i < scope.size(); i++) {
        Variable *x     = scope[i];
        int       xmin  = x->minimum();
        int       xmax  = x->maximum();
        int       coeff = coefficients[i];
        min += coeff * (coeff >= 0 ? xmin : xmax);
        max += coeff * (coeff >= 0 ? xmax : xmin);
    }
}


//----------------------------------------------
// Objective constraint
//----------------------------------------------

void SumGE::updateBound(long bound) { limit = bound; }   // Update the current bound

long SumGE::maxUpperBound() {
    computeBounds();
    return max;
}

long SumGE::minLowerBound() {
    computeBounds();
    return min;
}


long SumGE::computeScore(vec<int> &solution) { return weightedSum(solution); }

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------


SumGE::SumGE(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &coefs, long l) : Sum(p, n, vars, coefs, l) {
    leftmostPositiveCoefficientPosition = coefficients.size();
    for(int i = coefficients.size() - 1; i >= 0; i--) {
        if(coefficients[i] < 0)
            break;
        leftmostPositiveCoefficientPosition = i;
    }
}
