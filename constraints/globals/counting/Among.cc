//
// Created by audemard on 23/04/25.
//

#include "Among.h"

#include <stdexcept>

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool Among::isSatisfiedBy(vec<int> &tuple) {
    int cnt = 0;
    for(int v : tuple)
        if(values.find(v) != values.end())
            cnt++;
    return cnt == k;
}


bool Among::isCorrectlyDefined() { return true; }


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool Among::filter(Variable *x) {
    int nGuaranteedVars = 0, nPossibleVars = 0;
    mixedVariables.clear();
    for(int i = 0; i < scope.size(); i++) {
        Variable *x                      = scope[i];
        bool      atLeastOnePresentValue = false, atLeastOneAbsentValue = false;
        for(int idx : x->domain) {
            int idv = x->domain.toVal(idv);
            if((atLeastOnePresentValue && atLeastOneAbsentValue))
                break;
            bool among             = values.find(idv) != values.end();
            atLeastOnePresentValue = atLeastOnePresentValue || among;
            atLeastOneAbsentValue  = atLeastOneAbsentValue || !among;
        }

        if(atLeastOnePresentValue) {
            nPossibleVars++;
            if(!atLeastOneAbsentValue && ++nGuaranteedVars > k)
                return false;   // inconsistency detected
            if(atLeastOneAbsentValue)
                mixedVariables.add(i);
        }
    }

    if(nGuaranteedVars == k) {
        for(int i : mixedVariables)
            for(int val : values) solver->delVal(scope[i], val);
        solver->entail(this);
        return true;
    }
    if(nPossibleVars < k)
        return false;
    if(nPossibleVars == k) {
        for(int i : mixedVariables)
            for(int idv : scope[i]->domain) {
                int v = scope[i]->domain.toVal(idv);
                if(values.find(v) == values.end())
                    solver->delVal(scope[i], v);
            }
        solver->entail(this);
        return true;
    }
    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

Among::Among(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &_values, int kk)
    : GlobalConstraint(p, n, "Among", vars), k(kk), mixedVariables(vars.size(), false) {
    for(int v : _values) values.insert(v);
    throw std::runtime_error("c Among not yet tested");
}
