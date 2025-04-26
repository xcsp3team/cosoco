//
// Created by audemard on 25/04/25.
//

#include "NoGood.h"

#include <Solver.h>

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool NoGood::isSatisfiedBy(vec<int> &tuple) {
    int nb = 0;
    for(int j = 0; j < tuple.size(); j++) {
        if((*tuples)[0][j] != scope[j]->domain.toIdv(tuple[j]))
            break;
        nb++;
    }
    return nb != scope.size();
}


bool NoGood::isCorrectlyDefined() { return true; }

//----------------------------------------------
// Main filtering method
//----------------------------------------------

bool NoGood::filter(Variable *x) {
    int posx   = toScopePosition(x->idx);
    int tuplex = (*tuples)[0][posx];

    if(x->containsIdv(tuplex) == false) {
        solver->entail(this);
        return true;
    }

    int nbsz1 = 0;
    int sz2   = -1;
    for(int i = 0; i < scope.size(); i++) {
        if(scope[i]->size() > 2)
            return true;
        if(scope[i]->size() == 1 && scope[i]->domain[0] == (*tuples)[0][i])
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
        solver->delVal(scope[sz2], (*tuples)[0][sz2]);
        solver->entail(this);
    }
    return true;
}

//----------------------------------------------
// Constructors and initialisation
//----------------------------------------------

NoGood::NoGood(Problem &p, std::string n, vec<Variable *> &vars) : Extension(p, n, vars, 1, true) { type = "NoGood"; }


NoGood::NoGood(Problem &p, std::string n, vec<Variable *> &vars, Matrix<int> *tuplesFromOtherConstraint)
    : Extension(p, n, vars, true, tuplesFromOtherConstraint) {
    type = "NoGood";
}
