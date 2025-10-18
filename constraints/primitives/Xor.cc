//
// Created by audemard on 15/10/2025.
//

#include "Xor.h"

#include "Solver.h"


using namespace Cosoco;


//----------------------------------------------------------
// check validity
//----------------------------------------------------------


bool Xor::isSatisfiedBy(vec<int> &tuple) {
    int tmp = 0;
    for(int v : tuple)
        if(v == 1)
            tmp++;
    return tmp % 2 == 1;
}


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

Variable *Xor::findAnotherSentinel() {
    for(Variable *x : scope)
        if(x != sentinel1 && x != sentinel2 && x->size() > 1)
            return x;
    return nullptr;
}

bool Xor::enforceSentinel(Variable *sentinel) {
    int tmp = 0;
    for(Variable *x : scope)
        if(x != sentinel && x->value() == 1)
            tmp++;
    return solver->assignToVal(sentinel, tmp % 2 == 0 ? 1 : 0);
}

bool Xor::filter(Variable *dummy) {
    if(sentinel1->size() == 1) {
        Variable *y = findAnotherSentinel();
        if(y == nullptr) {
            if(enforceSentinel(sentinel2) == false)
                return false;
            return solver->entail(this);
        }
        sentinel1 = y;
    }
    if(sentinel2->size() == 1) {
        Variable *y = findAnotherSentinel();
        if(y == nullptr) {
            if(enforceSentinel(sentinel1) == false)
                return false;
            return solver->entail(this);
        }
        sentinel2 = y;
    }
    return true;
}


//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------
Xor::Xor(Problem &p, std::string n, vec<Variable *> &vars) : GlobalConstraint(p, n, "XOR", vars) {
    sentinel1 = vars[0];
    sentinel2 = vars[1];
}
