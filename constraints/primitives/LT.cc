#include "LT.h"

#include "solver/Solver.h"

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool Lt::isSatisfiedBy(vec<int> &tuple) { return tuple[0] + k < tuple[1]; }


//----------------------------------------------
// Filtering
//----------------------------------------------

bool Lt::filter(Variable *dummy) {
    if(solver->isAssigned(x) == false)
        if(solver->delValuesGreaterOrEqualThan(x, y->maximum() - k) == false)
            return false;

    if(solver->isAssigned(y) == false)
        if(solver->delValuesLowerOrEqualThan(y, x->minimum() + k) == false)
            return false;
    return true;
}


//----------------------------------------------
// Construction and initalisation
//----------------------------------------------

Lt::Lt(Problem &p, std::string n, Variable *xx, Variable *yy, int kk) : Binary(p, n, xx, yy), k(kk) { type = "(X - Y < k)"; }
