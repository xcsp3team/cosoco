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

void CumulativeWidthVariables::filterWidthVariables(vec<Variable *> &_heights) {
    if(timetableReasoner.nSlots > 0)
        for(int i = 0; i < starts.size(); i++) {
            if(widthVariables[i]->size() == 1)
                continue;
            int gap = widthVariables[i]->maximum() - widthVariables[i]->minimum();
            int ms1 = timetableReasoner.mandatoryStart(i), me1 = timetableReasoner.mandatoryEnd(i), me2 = me1 + gap;
            if(me2 <= ms1)
                continue;   // no mandatory part here
            int ms2            = me1 >= ms1 ? me1 : ms1;
            int virtual_height = wheights[i];   // height of the new "virtual" task (from ms2 to me2)
            for(int k = 0; k < timetableReasoner.nSlots; k++) {
                Slot slot = timetableReasoner.slots[k];
                if(slot.height + virtual_height - limit <= 0)
                    break;                                    // because we can no more find a conflict
                if(!(me2 <= slot.start || slot.end <= ms2))   // if overlapping
                    // widths[i].dom.removeValue(widths[i].dom.lastValue());
                    solver->delValuesGreaterOrEqualThan(widthVariables[i], widthVariables[i]->maximum() - (me2 - slot.start) + 1);
                // no possible conflict
            }
        }
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
