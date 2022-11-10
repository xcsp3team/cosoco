#include "MinimumConstantGE.h"

#include "solver/Solver.h"

using namespace Cosoco;


State MinimumConstantGE::status() { return solver->threadsGroup == nullptr && done ? CONSISTENT : UNDEF; }


void MinimumConstantGE::reinitialize() { done = false; }

//----------------------------------------------
// Check validity
//----------------------------------------------

bool MinimumConstantGE::isSatisfiedBy(vec<int> &tuple) {
    int min = tuple[0];
    for(int i = 0; i < tuple.size(); i++)
        if(tuple[i] < min)
            min = tuple[i];
    return min >= k;
}


//----------------------------------------------
// Filtering
//----------------------------------------------

bool MinimumConstantGE::filter(Variable *dummy) {
    for(int i = 0; i < scope.size(); i++)
        if(solver->delValuesLowerOrEqualThan(scope[i], k - 1) == false)
            return false;
   solver->entail(this);
    return true;
}
