//
// Created by audemard on 10/12/2020.
//

#include "HeuristicVarCACD.h"

#include "solver/Solver.h"
using namespace Cosoco;


HeuristicVarCACD::HeuristicVarCACD(Solver &s) : HeuristicVar(s) {
    s.addObserverConflict(this);
    s.addObserverDeleteDecision(this);
    s.addObserverNewDecision(this);
    vscores.growTo(solver.problem.nbVariables(), 0);
    cscores.growTo(solver.problem.nbConstraints());
    cvscores.growTo(solver.problem.nbConstraints());
    for(int i = 0; i < cvscores.size(); i++) cvscores[i].growTo(solver.problem.constraints[i]->scope.size());
}


void HeuristicVarCACD::reset() {
    vscores.fill(0);
    cscores.fill(0);
    for(auto &v : cvscores) v.fill(0);
}

Variable *HeuristicVarCACD::select() {
    if(solver.warmStart && solver.conflicts == 0 && solver.nbSolutions == 0) {
        for(int i = 0; i < solver.decisionVariables.size(); i++)
            if(solver.decisionVariables[i]->_name.rfind("__av", 0) != 0)
                return solver.decisionVariables[i];
    }

    if(solver.warmStart == false && solver.statistics[restarts] < 1 && solver.nbSolutions == 0) {
        auto &c = solver.problem.constraints.back();
        for(Variable *x : c->scope)
            if(x->size() > 1)
                return x;
    }


    Variable *x = solver.decisionVariables[0];


    double bestV = vscores[x->idx];
    for(int i = 1; i < solver.decisionVariables.size(); i++) {
        Variable *y = solver.decisionVariables[i];
        if(vscores[y->idx] > bestV) {
            bestV = vscores[y->idx];
            x     = y;
        }
    }
    assert(x != nullptr);
    return x;
}


void HeuristicVarCACD::notifyConflict(Constraint *c, int level) {
    if(freezed)
        return;
    double increment = 1;
    cscores[c->idc] += increment;   // just +1 in that case (can be useful for other objects, but not directly for wdeg)

    for(int i = 0; i < c->unassignedVariablesIdx.size(); i++) {
        Variable *y = c->scope[c->unassignedVariablesIdx[i]];
        increment   = 1.0 / (c->unassignedVariablesIdx.size() * (y->size() == 0 ? 0.5 : y->size()));

        vscores[y->idx] += increment;
        cvscores[c->idc][c->unassignedVariablesIdx[i]] += increment;
    }
}


void HeuristicVarCACD::notifyNewDecision(Variable *x, Solver &s) {
    if(freezed)
        return;

    for(Constraint *c : x->constraints)
        if(c->unassignedVariablesIdx.size() == 1) {
            int posy = c->unassignedVariablesIdx[0];   // the other variable whose score must be updated
            vscores[posy] -= cvscores[c->idc][posy];
        }
}


void HeuristicVarCACD::notifyDeleteDecision(Variable *x, int v, Solver &s) {
    if(freezed)
        return;

    for(Constraint *c : x->constraints)
        if(c->unassignedVariablesIdx.size() == 2) {
            int posy = c->unassignedVariablesIdx[0];   // the other variable whose score must be updated
            vscores[posy] += cvscores[c->idc][posy];
        }
}


void HeuristicVarCACD::notifyFullBacktrack() {
    if(freezed)
        return;

    if(solver.statistics[GlobalStats::restarts] > 0 &&
       ((solver.statistics[GlobalStats::restarts] + 1) - solver.lastSolutionRun) % 30 == 0)
        reset();
}
