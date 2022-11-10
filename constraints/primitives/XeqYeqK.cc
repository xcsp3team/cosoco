//
// Created by audemard on 01/02/2021.
//

#include "XeqYeqK.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

// checking
bool XeqYeqK::isSatisfiedBy(vec<int> &tuple) { return tuple[0] == (tuple[1] == k); }


//----------------------------------------------
// Filtering
//----------------------------------------------

// filtering
bool XeqYeqK::filter(Variable *dummy) {
    if(x->size() == 1) {
        if(x->value() == 0) {
            if(solver->delVal(y, k) == false)
                return false;
            solver->entail(this);
            return true;
        }
        if(solver->assignToVal(y, k) == false)
            return false;
        solver->entail(this);
        return true;
    }
    if(y->containsValue(k) == false) {
        if(solver->assignToVal(x, 0) == false)
            return false;
        solver->entail(this);
        return true;
    }

    if(y->size() == 1) {
        if(solver->assignToVal(x, y->value() == k) == false)
            return false;
        solver->entail(this);
        return true;
    }
    return true;
}


//----------------------------------------------
// Construction and initalisation
//----------------------------------------------

XeqYeqK::XeqYeqK(Problem &p, std::string n, Variable *xx, Variable *yy, int _k) : Binary(p, n, xx, yy), k(_k) {
    type = "X = Y = k";
}
