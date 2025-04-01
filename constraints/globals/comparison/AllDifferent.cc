#include "AllDifferent.h"

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

bool AllDifferentAC::filter(Variable *x) {
    if(matcher->findMaximumMatching() == false)
        return false;

    /*if(x->size() == 1) {
        // return true;
        int v = x->value();
        for(Variable *y : scope) {
            if(y == x)
                continue;
            if(solver->delVal(y, v) == false)
                return false;
        }
    }*/


    matcher->removeInconsistentValues();   // no more possible failure at this step
    // return true;

    return true;
}


//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------
AllDifferent::AllDifferent(Problem &p, std::string n, vec<Variable *> &vars) : GlobalConstraint(p, n, "All Different", vars) { }

AllDifferentWeak::AllDifferentWeak(Problem &p, std::string nn, vec<Variable *> &vars) : AllDifferent(p, nn, vars) {
    type = "All Different Weak";
}

AllDifferentAC::AllDifferentAC(Problem &p, std::string nn, vec<Variable *> &vars) : AllDifferent(p, nn, vars) {
    type    = "All Different AC";
    matcher = new Matcher(this);
}

void AllDifferentAC::attachSolver(Solver *s) {
    Constraint::attachSolver(s);
    s->addObserverDeleteDecision(matcher);   // We need to restore tuples.
}