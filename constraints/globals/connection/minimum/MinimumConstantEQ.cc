//
// Created by audemard on 24/11/2025.
//

#include "MinimumConstantEQ.h"

#include "Solver.h"
using namespace Cosoco;

//----------------------------------------------
// Check validity
//----------------------------------------------
bool MinimumConstantEQ::isSatisfiedBy(vec<int> &tuple) {
    int min = tuple[0];
    for(int v : tuple)
        if(v < min)
            min = v;
    return min == k;
}


//----------------------------------------------
// Filtering
//----------------------------------------------

Variable *MinimumConstantEQ::findNewSentinel() {
    for(Variable *x : scope)
        if(x != sentinel1 && x != sentinel2 && x->containsValue(k))
            return x;
    return nullptr;
}


bool MinimumConstantEQ::filter(Variable *dummy) {
    if(solver->decisionLevel() == 0) {
        for(Variable *x : scope)
            if(solver->delValuesLowerOrEqualThan(x, k - 1) == false)
                return false;
    }
    if(sentinel1->containsValue(k) == false) {
        Variable *sentinel = findNewSentinel();
        if(sentinel != nullptr)
            sentinel1 = sentinel;
        else {
            if(solver->assignToVal(sentinel2, k) == false)
                return false;
            return solver->entail(this);
        }
    }
    if(sentinel2->containsValue(k) == false) {
        Variable *sentinel = findNewSentinel();
        if(sentinel != nullptr)
            sentinel2 = sentinel;
        else {
            if(solver->assignToVal(sentinel1, k) == false)
                return false;
            return solver->entail(this);
        }
    }
    return true;
}
