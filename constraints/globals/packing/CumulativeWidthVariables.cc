//
// Created by audemard on 21/01/23.
//

#include "CumulativeWidthVariables.h"

#include "Cumulative.h"
#include "CumulativeHeightVariable.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool CumulativeWidthVariables::isSatisfiedBy(vec<int> &tuple) {
    vec<int> st;
    int      i, j = 0;
    for(i = 0; i < starts.size(); i++) st.push(tuple[i]);
    for(; i < tuple.size(); i++) wwidths[j++] = tuple[i];
    return Cumulative::isSatisfiedBy(st);
}


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool CumulativeWidthVariables::filter(Variable *dummy) {
    for(int i = 0; i < starts.size(); i++) wwidths[i] = widthVariables[i]->minimum();
    if(Cumulative::filter(dummy) == false)
        return false;
    filterWidthVariables(widthVariables);
    return true;
}

int CumulativeWidthVariables::maxWidth(int posx) { return widthVariables[posx]->maximum(); }

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

CumulativeWidthVariables::CumulativeWidthVariables(Problem &p, std::string n, vec<Variable *> &vars, vec<Variable *> &scope,
                                                   vec<Variable *> &widths, vec<int> &heights, int limit)
    : Cumulative(p, n, vars, scope, heights, heights, limit) {
    widths.copyTo(widthVariables);
    wwidths.growTo(widths.size(), 0);   // TODO: not so beautiful this constructor
}
