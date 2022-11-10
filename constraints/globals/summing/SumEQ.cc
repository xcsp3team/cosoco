#include "SumEQ.h"

#include "Sum.h"
#include "constraints/Constraint.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool SumEQ::isSatisfiedBy(vec<int> &tuple) { return weightedSum(tuple) == limit; }


bool Sum::isCorrectlyDefined() {
    if(scope.size() != coefficients.size())
        throw std::logic_error("Constraint " + std::to_string(idc) + ": Sum, list and coefficients mus have same size");

    for(int i = 1; i < coefficients.size(); i++) {
        if(coefficients[i] == 0)
            throw std::logic_error("Constraint " + std::to_string(idc) + ": Sum, a coefficient is 0");
        if(coefficients[i - 1] > coefficients[i]) {
            throw std::logic_error("Constraint " + std::to_string(idc) + ": Sum needs order on coefficients");
        }
    }
    return true;
}


//----------------------------------------------
// Filtering
//----------------------------------------------


bool SumEQ::filter(Variable *x) {
    long min = 0, max = 0;
    for(int i = 0; i < scope.size(); i++) {
        int c    = coefficients[i];
        int vmin = scope[i]->minimum();
        int vmax = scope[i]->maximum();
        if(c >= 0) {
            min += vmin * c;
            max += vmax * c;
        } else {
            min += vmax * c;
            max += vmin * c;
        }
    }
    if(min > limit || max < limit)
        return false;
    int positionOfLastChange = -1;
    while(true) {
        for(int i = 0; i < scope.size(); i++) {
            if(i == positionOfLastChange) {
                positionOfLastChange = -1;
                break;
            }
            Variable *x  = scope[i];
            int       sz = x->size();
            if(sz == 1)
                continue;
            int c = coefficients[i];
            if(c > 0) {
                min -= x->minimum() * c;
                max -= x->maximum() * c;
            } else {
                min -= x->maximum() * c;
                max -= x->minimum() * c;
            }
            for(int idv : reverse(x->domain)) {
                int v = x->domain.toVal(idv);
                if(max + v * c < limit && solver->delIdv(x, idv) == false)
                    return false;
                if(min + v * c > limit && solver->delIdv(x, idv) == false)
                    return false;
            }
            if(c > 0) {
                min += x->minimum() * c;
                max += x->maximum() * c;
            } else {
                min += x->maximum() * c;
                max += x->minimum() * c;
            }
            if(sz != x->size())
                positionOfLastChange = i;
        }
        if(positionOfLastChange == -1)
            break;
    }

    return true;
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------


SumEQ::SumEQ(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &coefs, long l) : Sum(p, n, vars, coefs, l) { }
