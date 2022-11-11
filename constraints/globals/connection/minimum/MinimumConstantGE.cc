#include "MinimumConstantGE.h"

#include "solver/Solver.h"

using namespace Cosoco;


State MinimumConstantGE::status() { return done ? CONSISTENT : UNDEF; }


void MinimumConstantGE::reinitialize() { done = false; }

//----------------------------------------------
// Check validity
//----------------------------------------------

bool MinimumConstantGE::isSatisfiedBy(vec<int> &tuple) {
    int min = tuple[0];
    for(int v : tuple)
        if(v < min)
            min = v;
    return min >= k;
}


//----------------------------------------------
// Filtering
//----------------------------------------------

bool MinimumConstantGE::filter(Variable *dummy) {
    for(auto &x : scope)
        if(solver->delValuesLowerOrEqualThan(x, k - 1) == false)
            return false;
    solver->entail(this);
    return true;
}
