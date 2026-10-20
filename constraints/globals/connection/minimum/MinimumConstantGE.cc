#include "MinimumConstantGE.h"

#include "solver/Solver.h"

using namespace Cosoco;


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
        if(solver->delValuesLE(x, k - 1) == false)
            return false;
    return solver->entail(this);
}
