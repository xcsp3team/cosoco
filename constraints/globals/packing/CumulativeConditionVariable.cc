//
// Created by audemard on 21/12/2022.
//

#include "CumulativeConditionVariable.h"

#include "Constraint.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool CumulativeConditionVariable::isSatisfiedBy(vec<int> &tuple) {
    limit = tuple.last();
    tuple.pop();
    bool tmp = Cumulative::isSatisfiedBy(tuple);
    tuple.push(limit);
    return tmp;
}


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool CumulativeConditionVariable::filter(Variable *dummy) {
    limit = limitVariable->maximum();
    if(Cumulative::filter(dummy) == false)
        return false;
    filterLimitVariable(limitVariable);
    return true;
}

void CumulativeConditionVariable::filterLimitVariable(Variable *x) {
    if(x->size() > 1 && timetableReasoner.nSlots > 0) {
        for(int idv : x->domain) {
            int v = x->domain.toVal(idv);
            if(timetableReasoner.slots[0].height > v)
                solver->delIdv(x, idv);   // No inconsistency
        }
    }
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

CumulativeConditionVariable::CumulativeConditionVariable(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &l,
                                                         vec<int> &h, Variable *_limit)
    : Cumulative(p, n, vars, Constraint::createScopeVec(&vars, _limit), l, h, 0) {
    limitVariable = _limit;
}
