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

bool XeqXor::isSatisfiedBy(vec<int> &tuple) {
    int tmp = 0;
    for(int i = 0; i < tuple.size() - 1; i++)
        if(tuple[i] == 1)
            tmp++;
    return tmp % 2 == tuple.last();
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
    for(Variable *x : scope)   // si eq=xor faut s'arrêter un avant la fin
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

Variable *XeqXor::findAnotherSentinel(Variable *sentinel) {
    for(Variable *x : vars)
        if(x != sentinel && x->size() > 1)
            return x;
    return nullptr;
}

int XeqXor::toBeRemoved(Variable *sentinel) {
    int cnt = 0;
    for(Variable *z : vars)
        if(z != sentinel && z->value() == 1)
            cnt++;
    return x->value() == 0 ? (cnt % 2 == 0 ? 1 : 0) : (cnt % 2 == 0 ? 0 : 1);
}

bool XeqXor::enforceSentinel(Variable *sentinel, int value) {
    int tmp = 0;
    for(Variable *x : scope)   // si eq=xor faut s'arrêter un avant la fin
        if(x != sentinel && x->value() == 1)
            tmp++;
    return solver->assignToVal(sentinel, tmp % 2 == 0 ? value : 1 - value);
}

bool XeqXor::filter(Variable *dummy) {
    if(x->size() == 2) {
        // only one sentinel is necessary for having AC
        if(sentinel1->size() == 2 || sentinel2->size() == 2)
            return true;
        Variable *y = findAnotherSentinel(sentinel2);
        if(y == nullptr) {
            int cnt = 0;
            for(Variable *z : vars)
                if(z->value() == 1)
                    cnt++;
            int v = cnt % 2 == 0 ? 1 : 0;
            return solver->delVal(x, v) && solver->entail(this);
        }
        sentinel1 = y;
        return true;
    }
    // if x=0 or x=1, we need two valid sentinels
    if(sentinel1->size() == 1) {
        Variable *y = findAnotherSentinel(sentinel2);
        if(y == nullptr) {
            int v = toBeRemoved(sentinel2);
            return solver->delVal(sentinel2, v) && solver->entail(this);
        }
        sentinel1 = y;
    }
    assert(sentinel1 != sentinel2);
    if(sentinel2->size() == 1) {
        Variable *y = findAnotherSentinel(sentinel1);
        if(y == nullptr) {
            int v = toBeRemoved(sentinel1);
            return solver->delVal(sentinel1, v) && solver->entail(this);
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

XeqXor::XeqXor(Problem &p, std::string n, vec<Variable *> &_vars, Variable *xx)
    : GlobalConstraint(p, n, "X = Xor", createScopeVec(&_vars, xx)), x(xx) {
    _vars.copyTo(vars);
    sentinel1 = vars[0];
    sentinel2 = vars[1];
}
