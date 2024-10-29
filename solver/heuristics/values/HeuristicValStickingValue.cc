#include "HeuristicValStickingValue.h"

#include "solver/Solver.h"

using namespace Cosoco;

HeuristicValStickingValue::HeuristicValStickingValue(Solver &s, std::unique_ptr<HeuristicVal> &&h)
    : HeuristicVal(s), hv(std::move(h)) {
    lastValue.growTo(solver.problem.nbVariables(), -1);
}


int HeuristicValStickingValue::select(Variable *x) {
    if(x->size() == 1) {
        lastValue[x->idx] = x->domain[0];
        return x->domain[0];
    }
    int lv = lastValue[x->idx];
    if(lv != -1 && x->domain.containsIdv(lv))
        return lv;
    return hv->select(x);
}
