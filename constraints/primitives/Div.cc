#include "Div.h"

#include "core/Variable.h"
#include "solver/Solver.h"

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool DivLE::isSatisfiedBy(vec<int> &tuple) { return tuple[0] / k <= tuple[1]; }
bool DivGE::isSatisfiedBy(vec<int> &tuple) { return tuple[0] / k >= tuple[1]; }

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool DivLE::filter(Variable *dummy) {   // x / k <= y
    return solver->delValuesLowerOrEqualThan(y, x->minimum() / k - 1) &&
           solver->delValuesGreaterOrEqualThan(x, y->maximum() * k + k);
}

bool DivGE::filter(Variable *dummy) {
    return solver->delValuesGreaterOrEqualThan(y, x->maximum() / k + 1) &&
           solver->delValuesLowerOrEqualThan(x, y->minimum() * k - 1);
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------
DivLE::DivLE(Problem &p, std::string n, Variable *xx, int kk, Variable *yy) : Binary(p, n, xx, yy), k(kk) { type = "x / k <= y"; }

DivGE::DivGE(Problem &p, std::string n, Variable *xx, int kk, Variable *yy) : Binary(p, n, xx, yy), k(kk) { type = "x / k >= y"; }
