#include "EQ.h"

#include "solver/Solver.h"

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool EQ::isSatisfiedBy(vec<int> &tuple) { return tuple[0] == tuple[1] + k; }

//----------------------------------------------
// Filtering
//----------------------------------------------

bool EQ::filter(Variable *xx) {
    if(x->size() == 1) {
        if(solver->assignToVal(y, x->value() - k) == false)
            return false;
        return true;
    }
    if(y->size() == 1) {
        if(solver->assignToVal(x, y->value() + k) == false)
            return false;
        return true;
    }


    for(int idv : reverse(y->domain)) {
        int v = y->domain.toVal(idv) + k;
        if(x->containsValue(v) == false && solver->delIdv(y, idv) == false)
            return false;
    }

    if(x->size() == y->size())
        return true;

    for(int idv : reverse(x->domain)) {
        int v = x->domain.toVal(idv) - k;
        if(y->containsValue(v) == false && solver->delIdv(x, idv) == false)
            return false;
    }


    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

EQ::EQ(Problem &p, std::string n, Variable *xx, Variable *yy, int _k) : Binary(p, n, xx, yy), k(_k) { type = "X = Y + k"; }
