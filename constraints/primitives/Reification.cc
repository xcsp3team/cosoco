//
// Created by audemard on 09/04/24.
//

#include "Reification.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool ReifLE::isSatisfiedBy(vec<int> &tuple) { return (tuple[0] == 1) == (tuple[1] <= tuple[2]); }


//----------------------------------------------
// Filtering
//----------------------------------------------


bool ReifLE::filter(Variable *dummy) {
    if(x->domain.size() == 1 && x->value() == 0) {   // Assigned at 0  => y > z
        if(solver->isAssigned(y) == false)
            if(solver->delValuesLowerOrEqualThan(y, z->minimum()) == false)
                return false;

        if(solver->isAssigned(z) == false)
            if(solver->delValuesGreaterOrEqualThan(z, y->maximum()) == false)
                return false;
        return true;
    }
    if(x->domain.size() == 1 && x->value() == 1) {   // Assigned at 0  => y > z
        if(solver->isAssigned(y) == false)
            if(solver->delValuesGreaterOrEqualThan(y, z->maximum() + 1) == false)
                return false;

        if(solver->isAssigned(z) == false)
            if(solver->delValuesLowerOrEqualThan(z, y->minimum() - 1) == false)
                return false;
        return true;
    }
    if(y->maximum() <= z->minimum())   // for sure x = 1
        return solver->assignToVal(x, 1);

    if(y->minimum() > z->maximum())   // for sure x = 0
        return solver->assignToVal(x, 0);
    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

ReifLE::ReifLE(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz) : Ternary(p, n, xx, yy, zz) {
    assert(xx->domain.maxSize() == 2);
    type = "X = (Y <= Z)";
}
