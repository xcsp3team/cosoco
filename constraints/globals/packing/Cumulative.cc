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
        if(tuple[i] + lengths[i] > max)
            max = tuple[i] + lengths[i];
    }

    for(int i = min; i < max; i++) {
        int h = 0;
        for(int t = 0; t < tuple.size(); t++)
            if(tuple[t] <= i && i < tuple[t] + lengths[t])
                h += heights[t];
        if(h > limit)
            return false;
    }
    return true;
    //    return IntStream.rangeClosed(min, max)
    //            .allMatch(t -> IntStream.range(0, tuple.length).filter(i -> tuple[i] <= t && t < tuple[i] + lengths[i]).map(i ->
    //            heights[i]).sum() <= limit);
}


bool Cumulative::isCorrectlyDefined() { return true; }


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

int Cumulative::buildTimeTable() {
    ticks.clear();

    for(int posx = 0; posx < variables.size(); posx++) {
        int ms = mandatoryStart(posx), me = mandatoryEnd(posx);
        if(me <= ms)
            continue;   // no mandatory part here
        if(ticks.contains(ms) == false) {
            ticks.add(ms);
            offsets[ms] = 0;
        }
        offsets[ms] += heights[posx];
        if(ticks.contains(me) == false) {
            ticks.add(me);
            offsets[me] = 0;
        }
        offsets[me] -= heights[posx];
    }
    // System.out.println(ticks);
    if(ticks.size() == 0)
        return 0;   // TRUE
    int nbRelevantTicks = 0;
    for(int i : ticks)
        if(offsets[i] != 0)
            slots[nbRelevantTicks++].start = i;


    sort(slots, nbRelevantTicks, CompareStart());
    nSlots = nbRelevantTicks - 1;
    // System.out.println("nSlots=" + nSlots);


    int h = 0;
    for(int k = 0; k < nSlots; k++) {
        slots[k].height = h + offsets[slots[k].start];
        if(slots[k].height > limit)
            return -1;   // Boolean.FALSE;
        slots[k].end = slots[k + 1].start;
        h            = slots[k].height;
    }

    sort(slots, nSlots, CompareHeight());

    return 1;
}


bool Cumulative::filter(Variable *dummy) {
    int b = buildTimeTable();
    if(b == -1)   // Conflict
        return false;
    if(b == 0)   // Nothing to do
        return true;

    for(int posx = 0; posx < variables.size(); posx++) {
        int ms = mandatoryStart(posx), me = mandatoryEnd(posx);
        for(int k = 0; k < nSlots; k++) {
            if(slots[k].height + heights[posx] <= limit)
                break;
            int rs = slots[k].start, re = slots[k].end;
            if(me <= ms) {   // if no mandatory part
                if(solver->delValuesInRange(variables[posx], rs - lengths[posx] + 1, re) == false)
                    return false;
            } else {
                if(me <= rs || re <= ms) {   // if the rectangle and the mandatory parts are disjoint
                    if(solver->delValuesInRange(variables[posx], rs - lengths[posx] + 1, re) == false)
                        return false;
                }
            }
        }
    }
    /*
     * int smin = Integer.MAX_VALUE, emax = -1;
    for(int j = futvars.limit; j >= 0; j--) {
        int i = futvars.dense[j];
        if(scp[i].dom.firstValue() < smin)
            smin = scp[i].dom.firstValue();
        if(emax < scp[i].dom.lastValue() + lengths[i])
            emax = scp[i].dom.lastValue() + lengths[i];
    }
    int depth = pb.solver.depth();
    for(int j = omega.limit; j >= 0; j--) {
        int i = omega.dense[j];
        if(scp[i].dom.size() == 1 && (scp[i].dom.lastValue() + lengths[i] <= smin || emax <= scp[i].dom.firstValue()))
            omega.removeAtPosition(j, depth);
    }*/
    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

int Cumulative::_horizon(Cosoco::vec<Cosoco::Variable *> &vars, vec<int> &l) {
    int h = -1;
    for(int i = 0; i < vars.size(); i++)
        if(vars[i]->maximum() + l[i] > h)
            h = vars[i]->maximum() + l[i];
    h++;

    return h;
}


Cumulative::Cumulative(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &l, vec<int> &h, int lm, Variable* limitV)
    : GlobalConstraint(p, n, "Cumulative", limitV == nullptr ? vars.size() : vars.size() + 1), ticks(_horizon(vars, l), false), omega(vars.size(), true) {
    limit = lm;

    vars.copyTo(variables);

    // Initialize in case condition is limit
    // TODO : Refactor... too ugly
    if(limitV != nullptr)
        vars.push(limitV);
    scopeInitialisation(vars);
    if(limitV != nullptr)
        vars.pop();

    l.copyTo(lengths);
    h.copyTo(heights);

    horizon = _horizon(vars, l);
    offsets.growTo(horizon);
    slots.growTo(horizon);
}
