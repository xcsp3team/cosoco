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

void CumulativeHeightVariable::filterHeightVariables(vec<Variable *> &_heights) {
    if(timetableReasoner.nSlots > 0) {
        for(int posx = 0; posx < starts.size(); posx++) {
            if(heightVariables[posx]->size() == 1)
                continue;
            int ms = timetableReasoner.mandatoryStart(posx), me = timetableReasoner.mandatoryEnd(posx);
            if(me <= ms)
                continue;   // no mandatory part here
            int increase = heightVariables[posx]->maximum() - heightVariables[posx]->minimum();
            for(int k = 0; k < timetableReasoner.nSlots; k++) {
                Slot slot    = timetableReasoner.slots[k];
                int  surplus = slot.height + increase - limit;
                if(surplus <= 0)
                    break;
                if(!(me <= slot.start || slot.end <= ms))   // if overlapping
                    solver->delValuesGreaterOrEqualThan(heightVariables[posx], heightVariables[posx]->maximum() - surplus + 1);
            }
        }
    }
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

CumulativeHeightVariable::CumulativeHeightVariable(Problem &p, std::string n, vec<Variable *> &vars, vec<Variable *> &scope,
                                                   vec<int> &l, vec<Variable *> &h, int _limit)
    : Cumulative(p, n, vars, scope, l, l, _limit) {
    h.copyTo(heightVariables);
    wheights.growTo(h.size(), 0);   // TODO: not so beautiful this constructor
}
