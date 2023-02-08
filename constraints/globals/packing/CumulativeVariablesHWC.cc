//
// Created by audemard on 23/01/23.
//

#include "CumulativeVariablesHWC.h"

#include "Cumulative.h"
using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool CumulativeVariablesHWC::isSatisfiedBy(vec<int> &tuple) {
    return true;
    vec<int> st;
    int      i, j = 0;
    for(i = 0; i < starts.size(); i++) st.push(tuple[i]);

    for(; i < starts.size() * 2; i++) wwidths[j++] = tuple[i];
    j = 0;
    for(; i < tuple.size(); i++) wheights[j++] = tuple[i];

    return Cumulative::isSatisfiedBy(st);
}


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool CumulativeVariablesHWC::filter(Variable *dummy) {
    for(int i = 0; i < starts.size(); i++) {
        wwidths[i]  = widthVariables[i]->minimum();
        wheights[i] = heightVariables[i]->minimum();
    }
    limit = limitVariable->maximum();


    if(Cumulative::filter(dummy) == false)
        return false;

    filterWidthVariables(widthVariables);
    filterHeightVariables(heightVariables);
    filterLimitVariable(limitVariable);
    return true;
}

int CumulativeVariablesHWC::maxWidth(int posx) { return widthVariables[posx]->maximum(); }

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------


CumulativeVariablesHWC::CumulativeVariablesHWC(Problem &p, std::string n, vec<Variable *> &vars, vec<Variable *> &w,
                                               vec<Variable *> &h, Variable *limit)
    : Cumulative(p, n, vars, Constraint::createScopeVec(&vars, &w, &h, limit), wwidths, wheights, limit->domain.maxSize()) {
    h.copyTo(heightVariables);
    w.copyTo(widthVariables);
    limitVariable = limit;
    wwidths.growTo(w.size(), 0);
    wheights.growTo(h.size(), 0);
}