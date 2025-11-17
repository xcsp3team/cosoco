#include "constraints/primitives/xAddyEqz.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool xAddyEQz::isSatisfiedBy(vec<int> &tuple) { return tuple[0] + tuple[1] == tuple[2]; }


//----------------------------------------------
// Filtering
//----------------------------------------------


bool xAddyEQz::filter(Variable *dummy) {
    if(x->size() == 1) {
        eq->x = z;
        eq->y = y;
        eq->k = x->value();
        return eq->filter(dummy);
    }
    if(y->size() == 1) {
        eq->x = z;
        eq->y = x;
        eq->k = y->value();
        return eq->filter(dummy);
    }
    if(z->size() == 1) {
        eq->x = z;
        eq->y = y;
        eq->k = x->value();
        return eq->filter(dummy);
    }


    if(y->size() == 1)
        return AC.enforceEQ(dz, dx, dy.singleValue());
    if(z->size() == 1)
        return AC.enforceEQb(dx, dy, dz.singleValue());
    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

xAddyEQz::xAddyEQz(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz) : Ternary(p, n, xx, yy, zz) {
    type = "X = Y + Z";
    eq   = new EQ(p, n, xx, yy);
}

void xAddyEQz::attachSolver(Solver *s) {
    Constraint::attachSolver(s);
    eq->attachSolver(s);
}
