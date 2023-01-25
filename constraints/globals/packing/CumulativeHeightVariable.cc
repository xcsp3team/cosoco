//
// Created by audemard on 21/01/23.
//

#include "CumulativeHeightVariable.h"

#include "Cumulative.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool CumulativeHeightVariable::isSatisfiedBy(vec<int> &tuple) {
    vec<int> st;
    int      i, j = 0;
    for(i = 0; i < starts.size(); i++) st.push(tuple[i]);
    for(; i < tuple.size(); i++) wheights[j++] = tuple[i];
    return Cumulative::isSatisfiedBy(st);
}


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool CumulativeHeightVariable::filter(Variable *dummy) {
    for(int i = 0; i < starts.size(); i++) wheights[i] = heightVariables[i]->minimum();
    if(Cumulative::filter(dummy) == false)
        return false;
    filterHeightVariables(heightVariables);
    return true;
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

CumulativeHeightVariable::CumulativeHeightVariable(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &l,
                                                   vec<Variable *> &h, int _limit)
    : Cumulative(p, n, vars, Constraint::createScopeVec(&vars, &h), l, l, _limit) {
    h.copyTo(heightVariables);
    wheights.growTo(h.size(), 0);
}
