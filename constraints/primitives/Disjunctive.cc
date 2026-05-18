#include "Disjunctive.h"

#include <algorithm>

#include "solver/Solver.h"
using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool Disjunctive::isSatisfiedBy(vec<int> &tuple) { return tuple[0] + lx <= tuple[1] || tuple[1] + ly <= tuple[0]; }

//----------------------------------------------
// Filtering
//----------------------------------------------

bool Disjunctive::filterDomain(Variable *z, int lbValue, int ubValue) {
    if(lbValue > ubValue)
        return true;   // nothing to filter

    if(z->size() == 1) {
        int v = z->value();
        return !(lbValue <= v && v <= ubValue);
    }
    for(int idv : reverse(z->domain)) {
        int v = z->domain.toVal(idv);
        if(v >= lbValue && v <= lbValue && solver->delVal(z, v) == false)
            return false;
    }
    return true;
}


bool Disjunctive::filter(Variable *xx) {
    if(x->maximum() + lx <= y->minimum()) {
        // x + wx <= y => z = 0
        if(solver->delVal(z, 1) == false)
            return false;
        return solver->entail(this);
    }
    if(y->maximum() + ly <= x->minimum()) {
        // y + wy <= x => z = 1
        if(solver->delVal(z, 0) == false)
            return false;
        return solver->entail(this);
    }

    if(z->maximum() == 0)   // z = 0 => x + wx <= y
        return solver->enforceLE(x, y, lx);
    if(z->minimum() == 1)   // z = 1 => y + wy <= x
        return solver->enforceLE(y, x, ly);

    if(solver->delValuesInRange(x, y->maximum() - lx + 1, y->minimum() + ly) == false)
        return false;
    if(solver->delValuesInRange(y, x->maximum() - ly + 1, x->minimum() + lx) == false)
        return false;
    return true;
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

Disjunctive::Disjunctive(Problem &p, std::string n, Variable *xx, Variable *yy, int ll1, int ll2, Variable *zz)
    : Ternary(p, n, xx, yy, zz), lx(ll1), ly(ll2) {
    type = "Disjunctive";
}
