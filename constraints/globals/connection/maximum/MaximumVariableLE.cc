#include "MaximumVariableLE.h"

#include "constraints/Constraint.h"
#include "solver/Solver.h"

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool MaximumVariableLE::isSatisfiedBy(vec<int>& tuple) {
    for(int i = 0; i < tuple.size() - 1; i++)   // Forget maxVar
        if(tuple[i] > maxVar->value())
            return false;
    return true;
}


//----------------------------------------------
// Filtering
//----------------------------------------------

bool MaximumVariableLE::filter(Variable* dummy) {
    int lb = maxVar->domain.minimum() - 1;
    int ub = lb;

    for(int i = 0; i < scope.size() - 1; i++) {   // Forget maxVar
        lb = std::max(lb, scope[i]->minimum());
        ub = std::max(ub, scope[i]->maximum());
    }
    // Update bounds of maxVars
    if(solver->delValuesLowerOrEqualThan(maxVar, lb) == false)
        return false;
    if(solver->delValuesGreaterOrEqualThan(maxVar, ub + 1) == false)
        return false;

    ub = maxVar->maximum();
    // Update scope vars
    for(int i = 0; i < scope.size() - 1; i++) {   // Forget maxVar
        if(solver->delValuesGreaterOrEqualThan(scope[i], ub + 1) == false)
            return false;
    }
    return true;
}