//
// Created by audemard on 24/05/2024.
//

#include "HeuristicVarFRBA.h"

#include "Options.h"
using namespace Cosoco;


HeuristicVarFRBA::HeuristicVarFRBA(Solver &s) : HeuristicVar(s) {
    s.addObserverConflict(this);
    s.addObserverDeleteDecision(this);
    s.addObserverNewDecision(this);
    data.growTo(s.problem.nbVariables(), {1, 2, 0});
    nFailedAssignments = 0;
}


Variable *HeuristicVarFRBA::select() {
    Variable *x    = solver.decisionVariables[0];
    double    best = data[x->idx](nFailedAssignments) / x->size();

    for(int i = 1; i < solver.decisionVariables.size(); i++) {
        Variable *y = solver.decisionVariables[i];
        if(data[y->idx](nFailedAssignments) / y->size() > best) {
            best = data[y->idx](nFailedAssignments) / y->size();
            x    = y;
        }
    }
    return x;
}

void HeuristicVarFRBA::notifyNewDecision(Cosoco::Variable *x, Cosoco::Solver &s) {
    if(freezed)
        return;
    data[x->idx].nAssignments++;
}


void HeuristicVarFRBA::notifyConflict(Constraint *c, int level) {
    if(freezed)
        return;
    if(solver.decisionLevel() == 0)
        return;
    Variable *last = solver.decisionVariableAtLevel(solver.decisionLevel());
    nFailedAssignments++;
    data[last->idx].nFailed++;
    data[last->idx].lastFailed = nFailedAssignments;
}


void HeuristicVarFRBA::notifyDeleteDecision(Variable *x, int v, Solver &s) { }


void HeuristicVarFRBA::notifyFullBacktrack() {
    if(options::boolOptions["rw"].value == false)
        return;
    if(freezed)
        return;

    if(solver.statistics[GlobalStats::restarts] > 0 &&
       ((solver.statistics[GlobalStats::restarts] + 1) - solver.lastSolutionRun) % 30 == 0) {
        printf("erer\n");
        data.fill({1, 2, 0});
    }
}
