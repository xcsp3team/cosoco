#include "MaximumVariableEQ.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity
//----------------------------------------------

bool MaximumVariableEQ::isSatisfiedBy(vec<int> &tuple) {
    int max = tuple[0];
    for(int i = 1; i < tuple.size() - 1; i++)
        if(tuple[i] > max)
            max = tuple[i];
    return max == tuple.last();
}


//----------------------------------------------
// Filtering
//----------------------------------------------

Variable *MaximumVariableEQ::findNewSentinelFor(int v, Variable *except) {
    for(Variable *x : list)
        if(x != except && x->containsValue(v))
            return x;
    return nullptr;
}


int MaximumVariableEQ::computeLimitForSentinel(Variable *sentinel) {
    int v = INT_MIN;

    for(int idv : value->domain) {
        int val = value->domain.toVal(idv);
        if((sentinels[idv] != sentinel || findNewSentinelFor(val, sentinel) != nullptr) && val >= v)
            v = val;
    }
    return v;
}


bool MaximumVariableEQ::filter(Variable *dummy) {
    int maxFirst = INT_MIN, maxLast = INT_MIN;
    for(Variable *x : list) {
        if(x->minimum() > maxFirst)
            maxFirst = x->minimum();
        if(x->maximum() > maxLast)
            maxLast = x->maximum();
    }

    // filtering the domain of the Min variable
    if(solver->delValuesGreaterOrEqualThan(value, maxLast + 1) == false ||
       solver->delValuesLowerOrEqualThan(value, maxFirst - 1) == false)
        return false;


    for(int idv : reverse(value->domain)) {
        int v = value->domain.toVal(idv);
        if(sentinels[idv]->containsValue(v) == false) {
            Variable *s = findNewSentinelFor(v, nullptr);
            if(s != nullptr)
                sentinels[idv] = s;
            else if(solver->delIdv(value, idv) == false)
                return false;
        }
    }

    // Filtering the domains of variables in the vector
    int max = value->maximum();
    for(Variable *x : list) {
        if(solver->delValuesGreaterOrEqualThan(x, max + 1) == false)
            return false;
    }

    // Possibly filtering the domain of the sentinel for the last value of the max variable
    Variable *sentinel = sentinels[value->domain.toIdv(value->maximum())];
    int       valLimit = computeLimitForSentinel(sentinel);
    if(valLimit == max)
        return true;   // because another sentinel exists
    for(int idv : reverse(sentinel->domain)) {
        int v = sentinel->domain.toVal(idv);
        if(v == max)
            continue;   // First element, skip it
        if(v <= valLimit)
            break;
        if(value->containsValue(v) == false && solver->delIdv(sentinel, idv) == false)
            return false;
    }

    return true;
}

//----------------------------------------------
// Constructor and initialisation methods
//----------------------------------------------

MaximumVariableEQ::MaximumVariableEQ(Problem &p, std::string n, vec<Variable *> &vars, Variable *v)
    : GlobalConstraint(p, n, "Maximum Variable", vars.size() + 1) {
    value = v;
    // Check if value is not in scope.
    for(Variable *x : vars) {
        if(x == value)
            throw std::runtime_error("Problem in definition of MaximumEQ Variable");
    }
    vars.push(value);   //
    scopeInitialisation(vars);
    vars.pop();   // Think to let vars unchanged.

    vars.copyTo(list);
    sentinels.growTo(value->domain.maxSize());
    for(int i = 0; i < value->domain.maxSize(); i++) sentinels[i] = findNewSentinelFor(value->domain.toVal(i), nullptr);
}