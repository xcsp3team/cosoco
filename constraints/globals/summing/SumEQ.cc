#include "SumEQ.h"

#include "Sum.h"
#include "constraints/Constraint.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool SumWeightedEQ::isSatisfiedBy(vec<int> &tuple) { return weightedSum(tuple) == limit; }

bool SumSimpleEQ::isSatisfiedBy(vec<int> &tuple) { return sum(tuple) == limit; }

//----------------------------------------------
// Filtering
//----------------------------------------------


bool SumWeightedEQ::filter(Variable *dummy) {
    computeBounds();
    if(min > limit || max < limit)
        return false;
    int positionOfLastChange = -1;
    if(min + maxGap <= limit && limit <= max - maxGap)
        return true;

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
            long c = coefficients[i];
            if(c > 0) {
                min -= x->minimum() * c;
                max -= x->maximum() * c;
            } else {
                min -= x->maximum() * c;
                max -= x->minimum() * c;
            }
            for(int idv : x->domain) {
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
    if(min == limit)
        solver->entail(this);
    return true;
}

bool SumSimpleEQ::filter(Variable *dummy) {
    computeBounds();
    if(min > limit || max < limit)
        return false;
    if(min + maxGap <= limit && limit <= max - maxGap)
        return true;


    int positionOfLastChange = -1;
    if(min + maxGap <= limit && limit <= max - maxGap)
        return true;

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
            min -= x->minimum();
            max -= x->maximum();
            if(solver->delValuesLE(x, limit - max - 1) == false || solver->delValuesGE(x, limit - min + 1) == false)
                return false;
            min += x->minimum();
            max += x->maximum();

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


SumWeightedEQ::SumWeightedEQ(Problem &p, vec<Variable *> &vars, vec<int> &coefs, long l) : WeightedSum(p, vars, coefs, l) {
    type = "SumWeightedEQ";
}

SumSimpleEQ::SumSimpleEQ(Problem &p, vec<Variable *> &vars, long l) : SimpleSum(p, vars, l) { type = "SumEQ"; }