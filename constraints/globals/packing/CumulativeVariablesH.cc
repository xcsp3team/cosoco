//
// Created by audemard on 21/01/23.
//

#include "CumulativeVariablesH.h"

#include "Cumulative.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool CumulativeVariablesH::isSatisfiedBy(vec<int> &tuple) {
    vec<int> st;
    int      i, j = 0;
    for(i = 0; i < starts.size(); i++) st.push(tuple[i]);
    for(; i < tuple.size(); i++) wheights[j++] = tuple[i];
    return Cumulative::isSatisfiedBy(st);
}

bool CumulativeVariablesHLimitV::isSatisfiedBy(vec<int> &tuple) {
    return true;   // TODO
}
//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool CumulativeVariablesH::filter(Variable *dummy) {
    for(int i = 0; i < starts.size(); i++) wheights[i] = heightVariables[i]->minimum();
    if(Cumulative::filter(dummy) == false)
        return false;
    filterHeightVariables(heightVariables);
    return true;
}

bool CumulativeVariablesHLimitV::filter(Variable *dummy) {
    for(int i = 0; i < starts.size(); i++) wheights[i] = heightVariables[i]->minimum();
    limit = limitvar->maximum();
    if(Cumulative::filter(dummy) == false)
        return false;
    filterHeightVariables(heightVariables);
    return true;
}
//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

CumulativeVariablesH::CumulativeVariablesH(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &l, vec<Variable *> &h,
                                           int _limit)
    : Cumulative(p, n, vars, Constraint::createScopeVec(&vars, &h), l, l, _limit) {
    h.copyTo(heightVariables);
    wheights.growTo(h.size(), 0);
}

CumulativeVariablesHLimitV::CumulativeVariablesHLimitV(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &l,
                                                       vec<Variable *> &h, Variable *_limit)
    : CumulativeVariablesH(p, n, vars, l, h, _limit->maximum()), limitvar(_limit) { }
