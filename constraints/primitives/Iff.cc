//
// Created by audemard on 18/02/2026.
//

#include "Iff.h"

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------
bool Iff::isSatisfiedBy(vec<int> &tuple) { return true; }

//----------------------------------------------
// Filtering
//----------------------------------------------

bool Iff::filter(Variable *dummy) {
    if(n1->maximum() == 0) {
        if(n2->setFalse(solver) == false)
            return false;
        return solver->entail(this);
    }
    if(n2->maximum() == 0) {
        if(n1->setFalse(solver) == false)
            return false;
        return solver->entail(this);
    }
    if(n1->minimum() == 1) {
        if(n2->setTrue(solver) == false)
            return false;
        return solver->entail(this);
    }

    if(n2->minimum() == 1) {
        if(n1->setTrue(solver) == false)
            return false;
        return solver->entail(this);
    }
    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------
Iff::Iff(Problem &p, std::string n, Variable *x1, Variable *x2, BasicNode *_n1, BasicNode *_n2) : Binary(p, n, x1, x2) {
    n1   = _n1;
    n2   = _n2;
    type = "Iff";
}
