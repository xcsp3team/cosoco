//
// Created by audemard on 14/02/23.
//

#include "PickOnDom.h"

using namespace Cosoco;


PickOnDom::PickOnDom(Solver &s) : HeuristicVar(s) {
    s.addObserverConflict(this);
    s.addObserverDeleteDecision(this);
    variablesWeights.growTo(s.problem.nbVariables(), 0);
}


Variable *PickOnDom::select() {
    Variable *x     = solver.decisionVariables[0];
    double    bestV = ((double)x->size()) / x->wdeg;
    for(int i = 1; i < solver.decisionVariables.size(); i++) {
        Variable *y = solver.decisionVariables[i];
        if(y->wdeg / ((double)y->size()) > bestV) {
            bestV = y->wdeg / ((double)y->size());
            x     = y;
        }
    }
    assert(x != nullptr);
    return x;
}


void PickOnDom::notifyConflict(Constraint *c, int level) {
    if(freezed)
        return;
    if(mode == 0) {
        for(PickVariables &p : solver.pickVariables) p.x->wdeg += 1;
        return;
    }
    if(mode == 3) {
        double ratio = 100.0 * (((double)solver.problem.nbVariables()) - ((double)solver.decisionLevel())) /
                       ((double)(solver.problem.nbVariables()));
        double sum = 0;
        for(PickVariables &p : solver.pickVariables) sum += p.deletedValues;
        for(PickVariables &p : solver.pickVariables) p.x->wdeg += ratio * (p.deletedValues / sum);
    }
    // for(int idx : solver.pickQueueHistory) solver.problem.variables[idx]->wdeg += solver.pickQueueHistory.counter(idx);
    //     for(int idx : solver.pickQueueHistory) std::cout << solver.pickQueueHistory.counter(idx) << " ";
    //     std::cout << std::endl;
}


void PickOnDom::notifyFullBacktrack() {
    if(freezed)
        return;
    if(solver.statistics[GlobalStats::restarts] > 0 &&
       ((solver.statistics[GlobalStats::restarts] + 1) - solver.lastSolutionRun) % 30 == 0) {
        printf("erer\n");
        for(Constraint *c : solver.problem.constraints) c->wdeg.fill(0);
        for(Variable *x : solver.problem.variables) x->wdeg = 0;
    }
}

bool PickOnDom::start() {
    freezed = false;
    for(int i = 0; i < variablesWeights.size(); i++) variablesWeights[i] = solver.problem.variables[i]->wdeg;
    return false;
}


bool PickOnDom::stop() {
    freezed = true;
    for(int i = 0; i < variablesWeights.size(); i++) solver.problem.variables[i]->wdeg = variablesWeights[i];
    return false;
}