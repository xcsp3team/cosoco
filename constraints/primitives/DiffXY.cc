#include "DiffXY.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool DiffXY::isSatisfiedBy(vec<int> &tuple) { return tuple[0] != tuple[1]; }


//----------------------------------------------
// Filtering
//----------------------------------------------

bool DiffXY::filter(Variable *xx) {
    if(x->size() > 1 && y->size() > 1)
        return true;

    if(x->size() == 1 && solver->delVal(y, x->value()) == false)
        return false;

    if(y->size() == 1 && solver->delVal(x, y->value()) == false)
        return false;

    return true;
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

DiffXY::DiffXY(Problem &p, std::string n, Variable *xx, Variable *yy) : Binary(p, n, xx, yy) { type = "(X != Y)"; }
