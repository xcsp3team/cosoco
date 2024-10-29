//
// Created by audemard on 21/12/2022.
//

#include "constraints/globals/packing/CumulativeVariablesC.h"

#include "constraints/Constraint.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool CumulativeVariablesC::isSatisfiedBy(vec<int> &tuple) {
    limit = tuple.last();
    tuple.pop();
    bool tmp = Cumulative::isSatisfiedBy(tuple);
    tuple.push(limit);
    return tmp;
}


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool CumulativeVariablesC::filter(Variable *dummy) {
    limit = limitVariable->maximum();
    if(Cumulative::filter(dummy) == false)
        return false;
    filterLimitVariable(limitVariable);
    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

CumulativeVariablesC::CumulativeVariablesC(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &l, vec<int> &h,
                                           Variable *_limit)
    : Cumulative(p, n, vars, Constraint::createScopeVec(&vars, _limit), l, h, 0) {
    limitVariable = _limit;
}
