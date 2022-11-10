#include "MinimumVariableEQ.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity
//----------------------------------------------

bool MinimumVariableEQ::isSatisfiedBy(vec<int> &tuple) {
    int min = tuple[0];
    for(int i = 1; i < tuple.size() - 1; i++)
        if(tuple[i] < min)
            min = tuple[i];
    return min == tuple.last();
}


//----------------------------------------------
// Filtering
//----------------------------------------------

Variable *MinimumVariableEQ::findNewSentinelFor(int v, Variable *except) {
    for(Variable *x : list)
        if(x != except && x->containsValue(v))
            return x;
    return nullptr;
}


int MinimumVariableEQ::computeLimitForSentinel(Variable *sentinel) {
    int v = INT_MAX;
    for(int idv : value->domain) {
        int val = value->domain.toVal(idv);
        if((sentinels[idv] != sentinel || findNewSentinelFor(val, sentinel) != nullptr) && val <= v)
            v = val;
    }
    return v;
    return INT_MAX;
}


bool MinimumVariableEQ::filter(Variable *dummy) {
    int minFirst = INT_MAX, minLast = INT_MAX;
    for(Variable *x : list) {
        if(x->minimum() < minFirst)
            minFirst = x->minimum();
        if(x->maximum() < minLast)
            minLast = x->maximum();
    }

    // filtering the domain of the Min variable
    if(solver->delValuesGreaterOrEqualThan(value, minLast + 1) == false ||
       solver->delValuesLowerOrEqualThan(value, minFirst - 1) == false)
        return false;


    for(int idv : reverse(value->domain)) {
        int v = value->domain.toVal(idv);
        if(sentinels[idv] == nullptr || sentinels[idv]->containsValue(v) == false) {
            Variable *s = findNewSentinelFor(v, nullptr);
            if(s != nullptr)
                sentinels[idv] = s;
            else if(solver->delIdv(value, idv) == false)
                return false;
        }
    }

    // Filtering the domains of variables in the vector
    int min = value->minimum();
    for(Variable *x : list) {
        if(solver->delValuesLowerOrEqualThan(x, min - 1) == false)
            return false;
    }

    // Possibly filtering the domain of the sentinel for the first value of the Min variable
    Variable *sentinel = sentinels[value->domain.toIdv(value->minimum())];
    int       valLimit = computeLimitForSentinel(sentinel);
    if(valLimit == min)
        return true;   // because another sentinel exists
    for(int idv : reverse(sentinel->domain)) {
        int v = sentinel->domain.toVal(idv);
        if(v == min)
            continue;   // First element, skip it
        if(v >= valLimit)
            break;
        if(value->containsValue(v) == false && solver->delIdv(sentinel, idv) == false)
            return false;
    }

    return true;
}

//----------------------------------------------
// Constructor and initialisation methods
//----------------------------------------------

MinimumVariableEQ::MinimumVariableEQ(Problem &p, std::string n, vec<Variable *> &vars, Variable *v)
    : GlobalConstraint(p, n, "Minimum Variable", vars.size() + 1) {
    value = v;
    // Check if value is not in scope.
    for(Variable *x : vars)
        if(x == value)
            throw std::runtime_error("Problem in definition of MinimumEQ Variable");

    vars.push(value);   //
    scopeInitialisation(vars);
    vars.pop();   // Think to let vars unchanged.

    vars.copyTo(list);
    sentinels.growTo(value->domain.maxSize());
    for(int i = 0; i < value->domain.maxSize(); i++) sentinels[i] = findNewSentinelFor(value->domain.toVal(i), nullptr);
}