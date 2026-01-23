#include "Add.h"

#include "solver/Solver.h"

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool Lt::isSatisfiedBy(vec<int> &tuple) { return tuple[0] + k < tuple[1]; }

bool Add3::isSatisfiedBy(vec<int> &tuple) { return tuple[0] + tuple[1] == tuple[2]; }

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

bool Add3::filter(Variable *dummy) {
    if(x->size() == 1)
        return solver->enforceEQ(z, y, x->value());
    if(y->size() == 1)
        return solver->enforceEQ(z, x, y->value());
    if(z->size() == 1)
        return solver->enforceAddEQ(x, y, z->value());

    if(tooLarge(x->size(), y->size())) {
        if(solver->delValuesLowerOrEqualThan(z, x->minimum() + y->minimum() - 1) == false)
            return false;
        if(solver->delValuesGreaterOrEqualThan(z, x->maximum() + y->maximum() + 1) == false)
            return false;
        if(solver->enforceAddGE(x, y, z->minimum()) == false)
            return false;
        if(solver->enforceAddLE(x, y, z->maximum()) == false)
            return false;
        if(tooLarge(x->size(), y->size()))   // otherwise we keep filtering below
            return true;
    }
 // TOO FINISH
    return true;
}

//----------------------------------------------
// Construction and initalisation
//----------------------------------------------

Lt::Lt(Problem &p, std::string n, Variable *xx, Variable *yy, int kk) : Binary(p, n, xx, yy), k(kk) { type = "(X - Y < k)"; }

Add3::Add3(Problem &p, std::string n, Variable *x, Variable *y, Variable *z) : Ternary(p, n, x, y, z) { type = "X + Y = Z"; }