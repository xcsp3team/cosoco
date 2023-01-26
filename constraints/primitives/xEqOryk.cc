//
// Created by audemard on 29/01/2021.
//

#include "xEqOryk.h"

#include "solver/Solver.h"

using namespace Cosoco;
//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool xEqOryk::isSatisfiedBy(vec<int> &tuple) {
    int r            = tuple.last();
    int clauseResult = 0;
    for(int i = 0; i < tuple.size() - 1; i++) {
        if(tuple[i] == values[i])
            clauseResult = 1;
    }

    return r == clauseResult;
}


//----------------------------------------------
// Filtering
//----------------------------------------------

bool xEqOryk::filter(Variable *x) {
    // useless ??
    if(solver->decisionLevel() == 0 && (result->size() > 2 || result->minimum() != 0 || result->maximum() != 1)) {
        for(int idv : result->domain) {
            int v = result->domain.toVal(idv);
            if(v != 0 && v != 1 && solver->delVal(x, idv) == false)
                return false;
        }
    }

    if(result->size() == 1) {
        if(result->value() == 0) {
            int i = 0;
            for(Variable *y : clause)
                if(solver->delVal(y, values[i++]) == false)
                    return false;
            solver->entail(this);
            return true;
        }
        assert(result->value() == 1);
        int nb  = 0;
        int pos = -1;
        int i   = 0;
        for(Variable *y : clause) {
            if(y->size() == 1 && y->value() == values[i]) {
                solver->entail(this);
                return true;
            }
            if(y->containsValue(values[i])) {
                nb++;
                if(pos == -1)
                    pos = i;
                else
                    break;
            }
            i++;
        }
        if(nb == 0)
            return false;
        if(nb == 1) {
            solver->assignToVal(clause[pos], values[pos]);
            solver->entail(this);
            return true;
        }
    }

    int i  = 0;
    int nb = 0;
    for(Variable *y : clause) {
        if(y->size() == 1 && y->value() == values[i]) {
            solver->assignToVal(result, 1);
            solver->entail(this);
            return true;
        }
        if(y->containsValue(values[i]))
            nb++;
        i++;
    }
    if(nb == 0) {
        solver->assignToVal(result, 0);
        solver->entail(this);
        return true;
    }
    return true;
}

//----------------------------------------------
// Constructor and initialisation methods
//----------------------------------------------

xEqOryk::xEqOryk(Problem &p, std::string n, Variable *r, vec<Variable *> &vars, vec<int> &_values)
    : GlobalConstraint(p, n, "X = Or(X1=k1, X2=k2...)", Constraint::createScopeVec(&vars, r)), result(r) {
    vars.copyTo(clause);
    _values.copyTo(values);
}
