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

bool Xor::enforceSentinel(Variable *sentinel, int value) {
    int tmp = 0;
    for(Variable *x : scope)   // si eq=xor faut s'arrêter un avant la fin
        if(x != sentinel && x->value() == 1)
            tmp++;
    return solver->assignToVal(sentinel, tmp % 2 == 0 ? value : 1 - value);
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

bool XeqXor::filter(Variable *dummy) {
    if(x->size() == 1) {
        if(sentinel1->size() == 1) {
            Variable *y = findAnotherSentinel();
            if(y == nullptr) {
                if(enforceSentinel(sentinel2, x->value()) == false)
                    return false;
                return solver->entail(this);
            }
            sentinel1 = y;
        }
        if(sentinel2->size() == 1) {
            Variable *y = findAnotherSentinel();
            if(y == nullptr) {
                if(enforceSentinel(sentinel1, x->value()) == false)
                    return false;
                return solver->entail(this);
            }
            sentinel2 = y;
        }
    }

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

XeqXor::XeqXor(Problem &p, std::string n, vec<Variable *> &vars, Variable *xx) : Xor(p, n, vars), x(xx) {
    sentinel1 = vars[0];
    sentinel2 = vars[1];
    addToScope(xx);
    type = "X = XOR";
}
