#include "DistNE.h"

#include "Binary.h"
#include "core/Variable.h"
#include "solver/Solver.h"


using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool DistNE::isSatisfiedBy(vec<int> &tuple) { return abs(tuple[0] - tuple[1]) != k; }

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool DistNE::filter(Variable *dummy) { return revise(x, y) && revise(y, x); }


bool DistNE::revise(Variable *z1, Variable *z2) {
    for(int idv1 : reverse(z1->domain))
        if(isASupportFor(z1->domain.toVal(idv1), z2) == false && solver->delIdv(z1, idv1) == false)
            return false;
    return true;
}


bool DistNE::isASupportFor(int vy, Variable *z) {
    for(int idv : z->domain)
        if(abs(vy - z->domain.toVal(idv)) != k)
            return true;
    return false;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------
DistNE::DistNE(Problem &p, std::string n, Variable *xx, Variable *yy, int kk) : Binary(p, n, xx, yy), k(kk) {
    type = "(|X - Y| != k)";
}
