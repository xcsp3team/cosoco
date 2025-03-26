#include "AllDifferentWeak.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------------------
// check validity
//----------------------------------------------------------


bool AllDifferent::isSatisfiedBy(vec<int> &tuple) {
    for(int posx = 0; posx < tuple.size(); posx++) {
        int v = tuple[posx];
        for(int posy = posx + 1; posy < tuple.size(); posy++)
            if(v == tuple[posy])
                return false;
    }
    return true;
}


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------


bool AllDifferentWeak::filter(Variable *x) {
    if(x->size() > 1)
        return true;
    int v = x->value();
    for(Variable *y : scope) {
        if(y == x)
            continue;
        if(solver->delVal(y, v) == false)
            return false;
    }
    return true;
}


//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------
AllDifferent::AllDifferent(Problem &p, std::string n, vec<Variable *> &vars) : GlobalConstraint(p, n, "All Different", vars) { }

AllDifferentWeak::AllDifferentWeak(Problem &p, std::string nn, vec<Variable *> &vars) : AllDifferent(p, nn, vars) {
    type = "All Different Weak";
}


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------
