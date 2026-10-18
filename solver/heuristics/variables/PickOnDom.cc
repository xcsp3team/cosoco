//
// Created by audemard on 14/02/23.
//

#include "PickOnDom.h"

#include "Options.h"

using namespace Cosoco;


PickOnDom::PickOnDom(Solver &s) : HeuristicVar(s) {
    s.addObserverConflict(this);
    s.addObserverDeleteDecision(this);
    variablesWeights.growTo(s.problem.nbVariables(), 1);
}


Variable *PickOnDom::select() {
    if(lazy && secondBest != nullptr && solver.unassignedVariables.contains(secondBest)) {
        Variable *tmp = secondBest;
        secondBest    = nullptr;
        return tmp;
    }

    Variable *x    = solver.decisionVariables[0];
    double    wdeg = variablesWeights[x->idx];

    double bestV = static_cast<double>(x->size()) / wdeg;
    for(int i = 1; i < solver.decisionVariables.size(); i++) {
        Variable *y     = solver.decisionVariables[i];
        double    ywdeg = variablesWeights[y->idx];

        if(ywdeg / static_cast<double>(y->size()) > bestV) {
            bestV      = ywdeg / static_cast<double>(y->size());
            secondBest = x;
            x          = y;
        } else {
            if(ywdeg / static_cast<double>(y->size()) == bestV)
                secondBest = y;
        }
    }
    assert(x != nullptr);
    return x;
}


void PickOnDom::notifyConflict(Constraint *c, int level) {
    secondBest = nullptr;
    if(freezed)
        return;
    if(mode == 0) {
        for(PickVariables &p : solver.pickVariables) variablesWeights[p.x->idx]++;
        return;
    }
    if(mode == 3) {
        double ratio = 100.0 * (((double)solver.problem.nbVariables()) - ((double)solver.decisionLevel())) /
                       ((double)(solver.problem.nbVariables()));
        double sum = 0;
        for(PickVariables &p : solver.pickVariables) sum += p.deletedValues;
        for(PickVariables &p : solver.pickVariables) variablesWeights[p.x->idx] += ratio * (p.deletedValues / sum);
    }
    // for(int idx : solver.pickQueueHistory) solver.problem.variables[idx]->wdeg += solver.pickQueueHistory.counter(idx);
    //     for(int idx : solver.pickQueueHistory) std::cout << solver.pickQueueHistory.counter(idx) << " ";
    //     std::cout << std::endl;
}


void PickOnDom::notifyFullBacktrack() {
    secondBest = nullptr;
    if(freezed)
        return;
    if(options::boolOptions["rw"].value == false)
        return;

    if(solver.statistics[GlobalStats::restarts] > 0 &&
       ((solver.statistics[GlobalStats::restarts] + 1) - solver.lastSolutionRun) % 30 == 0) {
        printf("erer\n");
        for(int i = 0; i < variablesWeights.size(); i++) variablesWeights[i] = 1;
    }
}
