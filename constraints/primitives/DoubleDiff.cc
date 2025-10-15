//
// Created by audemard on 15/10/2025.
//

#include "DoubleDiff.h"

#include "Solver.h"
#include "Xor.h"


using namespace Cosoco;


//----------------------------------------------------------
// check validity
//----------------------------------------------------------


bool DoubleDiff::isSatisfiedBy(vec<int> &tuple) { return tuple[0] != tuple[1] || tuple[2] != tuple[3]; }


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

int DoubleDiff::findAnotherSentinel() {
    for(int i = 0; i < scope.size(); i++)
        if(i != sentinel1 && i != sentinel2 && scope[i]->size() > 1)
            return i;
    return -1;
}

bool DoubleDiff::enforceSentinel(int sentinel) {
    assert((scope[0]->size() > 1 ? 1 : 0) + (scope[1]->size() > 1 ? 1 : 0) + (scope[2]->size() > 1 ? 1 : 0) +
               (scope[3]->size() > 1 ? 1 : 0) <=
           1);
    Variable *x = scope[sentinel];
    if(x->size() == 1)   // all 4 variables assigned
        return scope[0]->value() != scope[1]->value() || scope[2]->value() != scope[3]->value();

    if(sentinel < 2) {
        if(scope[2]->value() != scope[3]->value())
            return true;
        int v = scope[sentinel == 0 ? 1 : 0]->value();
        return solver->delVal(x, v);   // no inconsistency possible here
    } else {
        if(scope[0]->value() != scope[1]->value())
            return true;
        int v = scope[sentinel == 2 ? 3 : 2]->value();
        return solver->delVal(x, v);   // no inconsistency possible here
    }
}

bool DoubleDiff::filter(Variable *dummy) {
    if(scope[sentinel1]->size() == 1) {
        int idx = findAnotherSentinel();
        if(idx == -1) {
            if(enforceSentinel(sentinel2) == false)
                return false;
            return solver->entail(this);
        }
        sentinel1 = idx;
    }
    if(scope[sentinel2]->size() == 1) {
        int idx = findAnotherSentinel();
        if(idx == -1) {
            if(enforceSentinel(sentinel1) == false)
                return false;
            return solver->entail(this);
        }
        sentinel2 = idx;
    }
    return true;
}


//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------
DoubleDiff::DoubleDiff(Problem &p, std::string n, Variable *xx1, Variable *xx2, Variable *yy1, Variable *yy2)
    : Constraint(p, n, createScopeVec(xx1, xx2, yy1, yy2)) {
    type      = "DoubleDiff";
    sentinel1 = 0;
    sentinel2 = 1;
}
