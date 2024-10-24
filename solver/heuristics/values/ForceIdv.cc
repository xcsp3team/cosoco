#include "ForceIdv.h"

#include <core/OptimizationProblem.h>

#include "solver/Solver.h"

using namespace Cosoco;

ForceIdvs::ForceIdvs(Solver &s, HeuristicVal *h, bool oo, vec<int> *values)
    : HeuristicVal(s), hv(h), conflictAlreadySeen(false), onlyOnce(oo) {
    s.addObserverConflict(this);
    idvs.growTo(solver.problem.nbVariables(), -1);
    if(values != nullptr) {
        int i = 0;
        assert(values->size() <= solver.problem.nbVariables());
        for(int v : *values) {
            idvs[i] = solver.problem.variables[i]->domain.toIdv(v);
            i++;
        }
    }
}

void ForceIdvs::notifyConflict(Constraint *c, int level) {
    return;
    if(onlyOnce == false)
        return;
    if(conflictAlreadySeen)
        return;
    idvs.fill(-1);
    conflictAlreadySeen = true;
}


int ForceIdvs::select(Variable *x) {
    if(x->size() == 1) {
        return x->domain[0];
    }
    if(isDisabled)
        return hv->select(x);

    int lv = idvs[x->idx];
    if(lv != -1 && x->domain.containsIdv(lv))
        return lv;
    return hv->select(x);
}

void ForceIdvs::setIdValues(vec<int> &v) {
    v.copyTo(idvs);
    conflictAlreadySeen = false;
}
