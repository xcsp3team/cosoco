//
// Created by audemard on 25/04/25.
//

#include "NoGood.h"

#include <Solver.h>

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool NoGood::isSatisfiedBy(vec<int> &t) {
    return true;   // TODO StillLife-wastage-05-05_c24
    for(int i = 0; i < tuple.size(); i++) {
        std::cout << tuple[i] << " \n";
        if(scope[i]->domain.toVal(tuple[i]) != t[i])
            return true;
    }
    return false;
}


bool NoGood::isCorrectlyDefined() { return true; }

//----------------------------------------------
// Main filtering method
//----------------------------------------------

bool NoGood::filter(Variable *x) {
    // int posx   = toScopePosition(x->idx);
    // int tuplex = tuple[posx];

    // if(x->containsValue(tuplex) == false) {
    //     solver->entail(this);
    //     return true;
    // }

    int nbsz1 = 0;
    int sz2   = -1;
    for(int i = 0; i < scope.size(); i++) {
        if(scope[i]->containsIdv(tuple[i]) == false) {
            solver->entail(this);
            return true;
        }
        if(scope[i]->size() > 2)
            return true;
        if(scope[i]->size() == 1 && scope[i]->domain[0] == tuple[i])
            nbsz1++;
        if(scope[i]->size() == 2) {
            if(sz2 != -1)
                return true;
            sz2 = i;
        }
    }
    if(nbsz1 == scope.size())
        return false;
    if(nbsz1 == scope.size() - 1 && sz2 != -1) {
        solver->delIdv(scope[sz2], tuple[sz2]);
        solver->entail(this);
    }
    return true;
}

//----------------------------------------------
// Constructors and initialisation
//----------------------------------------------

NoGood::NoGood(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &t) : Constraint(p, n, vars) {
    type = "NoGood";
    t.copyTo(tuple);
}
