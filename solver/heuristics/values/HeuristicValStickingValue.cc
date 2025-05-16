#include "HeuristicValStickingValue.h"

#include "solver/Solver.h"

using namespace Cosoco;

HeuristicValStickingValue::HeuristicValStickingValue(Solver &s, HeuristicVal *h) : HeuristicVal(s), hv(h) {
    lastValue.growTo(solver.problem.nbVariables(), -1);
    s.addObserverSingletonVariable(this);
}

void HeuristicValStickingValue::notifySingletonVariable(Variable *x) { lastValue[x->idx] = x->domain[0]; }

int HeuristicValStickingValue::select(Variable *x) {
    int lv = lastValue[x->idx];
    if(lv != -1 && x->domain.containsIdv(lv))
        return lv;
    return hv->select(x);
}
