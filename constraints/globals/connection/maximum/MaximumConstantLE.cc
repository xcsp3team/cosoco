#include "MaximumConstantLE.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Constraint done after first filtering
//----------------------------------------------


State MaximumConstantLE::status() { return solver->threadsGroup == nullptr && done ? CONSISTENT : UNDEF; }


void MaximumConstantLE::reinitialize() { done = false; }

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool MaximumConstantLE::isSatisfiedBy(vec<int> &tuple) {
    int max = tuple[0];
    for(int i = 1; i < tuple.size(); i++)
        if(tuple[i] > max)
            max = tuple[i];
    return max <= k;
}


//----------------------------------------------
// Filtering
//----------------------------------------------

bool MaximumConstantLE::filter(Variable *dummy) {
    for(int i = 0; i < scope.size(); i++)
        if(solver->delValuesGreaterOrEqualThan(scope[i], k + 1) == false)
            return false;
    solver->entail(this);
    return true;
}
