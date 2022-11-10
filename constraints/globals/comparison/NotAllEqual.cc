#include "NotAllEqual.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------------------
// check validity and correct definition
//----------------------------------------------------------

bool NotAllEqual::isSatisfiedBy(vec<int> &tuple) {
    for(int i = 0; i < tuple.size() - 1; i++)
        if(tuple[i] != tuple[i + 1])
            return true;
    return false;
}

//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool NotAllEqual::filter(Variable *dummy) {
    Variable *x = nullptr;
    int       v = INT_MAX;

    // iteration on future variables first
    for(int idx : unassignedVariablesIdx) {
        Variable *tmp = scope[idx];
        if(tmp->size() > 1) {
            if(x == nullptr)
                x = tmp;
            else
                return true;   // Two variables have size>1: no problem
        } else {
            if(v == INT_MAX)
                v = tmp->value();
            else if(v != tmp->value()) {   // Two variables with different values: no problem
                solver->entail(this);
                return true;
            }
        }
    }
    // iteration on past variable
    for(int i = unassignedVariablesIdx.size(); i < unassignedVariablesIdx.maxSize(); i++) {
        Variable *tmp = scope[unassignedVariablesIdx[i]];
        assert(tmp->size() <= 1);
        if(v == INT_MAX)
            v = tmp->value();
        else if(v != tmp->value()) {   // We found two variables with different values: no problem
            solver->entail(this);
            return true;
        }
    }

    if(x == nullptr)
        return false;
    assert(v != INT_MAX);
    solver->entail(this);
    return solver->delVal(x, v);
}


//----------------------------------------------
// Constructor and initialisation methods
//----------------------------------------------

NotAllEqual::NotAllEqual(Problem &p, std::string nn, vec<Variable *> &vars) : GlobalConstraint(p, nn, "Not All Equal", vars) { }
