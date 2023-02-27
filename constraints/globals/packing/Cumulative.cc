#include "Cumulative.h"

#include <mtl/Sort.h>

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool Cumulative::isSatisfiedBy(vec<int> &tuple) {
    int min = horizon;
    int max = -1;
    for(int i = 0; i < tuple.size(); i++) {
        if(tuple[i] < min)
            min = tuple[i];
        if(tuple[i] + wwidths[i] > max)
            max = tuple[i] + wwidths[i];
    }

    for(int i = min; i < max; i++) {
        int h = 0;
        for(int t = 0; t < tuple.size(); t++)
            if(tuple[t] <= i && i < tuple[t] + wwidths[t])
                h += wheights[t];
        if(h > limit)
            return false;
    }
    return true;
}


bool Cumulative::isCorrectlyDefined() { return true; }


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

int TimeTableReasoner::mandatoryStart(int i) { return cumulative.starts[i]->maximum(); }
int TimeTableReasoner::mandatoryEnd(int i) { return cumulative.starts[i]->minimum() + cumulative.wwidths[i]; }


int TimeTableReasoner::buildSlots() {
    nSlots = 0;
    ticks.clear();
    for(int posx = 0; posx < cumulative.starts.size(); posx++) {
        int ms = mandatoryStart(posx), me = mandatoryEnd(posx);
        if(me <= ms)
            continue;   // no mandatory part here
        if(ticks.contains(ms) == false) {
            ticks.add(ms);
            offsets[ms] = 0;
        }
        offsets[ms] += cumulative.wheights[posx];
        if(ticks.contains(me) == false) {
            ticks.add(me);
            offsets[me] = 0;
        }
        offsets[me] -= cumulative.wheights[posx];
    }
    int nRelevantTicks = 0;
    for(int tick : ticks)
        if(offsets[tick] != 0)   // ticks with offset at 0 are not relevant (and so, are discarded)
            slots[nRelevantTicks++].start = tick;
    if(nRelevantTicks == 0)
        return 1;

    sort(slots, nRelevantTicks, CompareStart());
    for(int k = 0, height = 0; k < nRelevantTicks - 1; k++) {
        height += offsets[slots[k].start];
        if(height > cumulative.limit)
            return 0;
        slots[k].end    = slots[k + 1].start;
        slots[k].height = height;
    }

    nSlots = nRelevantTicks - 1;
    sort(slots, nSlots, CompareHeight());

    while(slots[nSlots - 1].height == 0) nSlots--;   // we discard irrelevant slots (those of height 0)
    return -1;
}

void TimeTableReasoner::updateRelevantTasks() {
    int level = cumulative.solver->decisionLevel();

    // we compute the relevant time bounds: minimum relevant start time and maximum relevant end time from
    // current future variables
    int smin = INT_MAX, emax = INT_MIN;
    for(int j = 0; j < cumulative.unassignedVariablesIdx.size(); j++) {
        int i = cumulative.unassignedVariablesIdx[j];
        if(i >= cumulative.starts.size())
            continue;
        smin = std::min(smin, cumulative.starts[i]->minimum());
        emax = std::max(emax, cumulative.starts[i]->maximum() + cumulative.maxWidth(i));
    }
    // we update the set of relevant tasks to consider from (smin,emax)
    for(int j = relevantTasks.size() - 1; j >= 0; j--) {
        int i = relevantTasks[j];
        if(cumulative.starts[i]->size() == 1 &&
           (cumulative.starts[i]->maximum() + cumulative.maxWidth(i) <= smin || emax <= cumulative.starts[i]->minimum()))
            relevantTasks.del(j, level);
    }
}


bool TimeTableReasoner::filter() {
    // Variable lastPast = problem.solver.futVars.lastPast();
    for(int posx = 0; posx < cumulative.starts.size(); posx++) {
        /*    int i = energeticReasoner.sortedTasks[j];
            if (slots[0].height + wheights[i] <= limit)
                break;
            if (starts[i].assigned() && starts[i] != lastPast)
                continue;
        */
        int ms = mandatoryStart(posx), me = mandatoryEnd(posx);
        for(int k = 0; k < nSlots; k++) {
            if(slots[k].height + cumulative.wheights[posx] <= cumulative.limit)
                break;
            assert(slots[k].height != 0);
            int rs = slots[k].start, re = slots[k].end;
            // if no mandatory part or if the rectangle and the mandatory parts are disjoint
            if(me <= ms || me <= rs || re <= ms) {
                if(cumulative.solver->delValuesInRange(cumulative.starts[posx], rs - cumulative.wwidths[posx] + 1, re) == false)
                    return false;
            }
        }
    }
    updateRelevantTasks();
    return true;
}
TimeTableReasoner::TimeTableReasoner(Cumulative &c) : cumulative(c), nSlots(0) { }

int Cumulative::maxWidth(int posx) { return wwidths[posx]; }

bool Cumulative::filter(Variable *dummy) {
    int b = timetableReasoner.buildSlots();
    if(b == 0)
        return false;   // seems better than x.dom.fail()
    if(b == 1)
        return true;

    /*b = energeticReasoner.filter();
    if (b == Boolean.FALSE)
        return false; // seems better than x.dom.fail()
    if (b == Boolean.TRUE)
        return true;
*/
    return timetableReasoner.filter();
}

//----------------------------------------------
// Observers methods
//----------------------------------------------


void Cumulative::notifyDeleteDecision(Variable *x, int v, Solver &s) {
    timetableReasoner.relevantTasks.restoreLimit(s.decisionLevel() + 1);
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

int Cumulative::_horizon(Cosoco::vec<Cosoco::Variable *> &vars) {
    int h = -1;
    for(int i = 0; i < vars.size(); i++)
        if(vars[i]->maximum() + maxWidth(i) > h)
            h = vars[i]->maximum() + maxWidth(i);
    h++;
    return h;
}


Cumulative::Cumulative(Problem &p, std::string n, vec<Variable *> &vars, vec<Variable *> &scope, vec<int> &l, vec<int> &h, int lm)
    : GlobalConstraint(p, n, "Cumulative", scope), timetableReasoner(*this) {
    limit = lm;

    vars.copyTo(starts);


    l.copyTo(wwidths);
    h.copyTo(wheights);
}

void Cumulative::attachSolver(Solver *s) {
    // Init timetableReasoner here..
    horizon = _horizon(starts);
    timetableReasoner.offsets.growTo(horizon);
    timetableReasoner.slots.growTo(horizon);
    timetableReasoner.ticks.setCapacity(horizon, false);
    timetableReasoner.relevantTasks.setCapacity(starts.size(), true);

    Constraint::attachSolver(s);
    s->addObserverDeleteDecision(this);   // We need to restore relevantTasks.
}


//-----------------------------------------------------------------------
// Usefull for children classes
void Cumulative::filterHeightVariables(vec<Variable *> &heightVariables) {
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


void Cumulative::filterWidthVariables(vec<Variable *> &widthVariables) {
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

void Cumulative::filterLimitVariable(Variable *x) {
    if(x->size() > 1 && timetableReasoner.nSlots > 0) {
        for(int idv : x->domain) {
            int v = x->domain.toVal(idv);
            if(timetableReasoner.slots[0].height > v)
                solver->delIdv(x, idv);   // No inconsistency
        }
    }
}
