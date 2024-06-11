//
// Created by audemard on 11/06/24.
//

#include "HeuristicVarCRBS.h"

#include "solver/Solver.h"

using namespace Cosoco;


HeuristicVarCRBS::HeuristicVarCRBS(Cosoco::Solver &s) : HeuristicVar(s) {
    s.addObserverConflict(this);
    s.addObserverDeleteDecision(this);
    s.addObserverNewDecision(this);
    a = new Matrix<int>(solver.problem.nbVariables(), solver.problem.nbVariables());
    for(int i = 0; i < solver.problem.nbVariables(); i++) a->fillRow(i, 0);
}


void HeuristicVarCRBS::notifyFullBacktrack() {
    if(freezed)
        return;
    if(solver.statistics[GlobalStats::restarts] > 0 &&
       ((solver.statistics[GlobalStats::restarts] + 1) - solver.lastSolutionRun) % 30 == 0) {
        printf("erer\n");
        for(int i = 0; i < solver.problem.nbVariables(); i++) a->fillRow(i, 0);
    }
}


void HeuristicVarCRBS::notifyConflict(Cosoco::Constraint *c, int level) {
    if(freezed)
        return;
    if(solver.decisionLevel() == 0)
        return;   // This is the end
    int idx = solver.decisionVariableAtLevel(solver.decisionLevel())->idx;
    for(int idy = 0; idy < solver.problem.nbVariables(); idy++) {
        if(idx == idy)
            (*a)[idx][idx] += 2;
        else {
            (*a)[idx][idy]++;
            (*a)[idy][idx]++;
        }
    }
}


void HeuristicVarCRBS::notifyNewDecision(Variable *x, Solver &s) {
    if(freezed)
        return;
    if(solver.decisionLevel() <= 1)
        return;   // First decision
    int idx = solver.decisionVariableAtLevel(solver.decisionLevel() - 1)->idx;
    for(int idy = 0; idy < solver.problem.nbVariables(); idy++) {
        if(idx == idy)
            (*a)[idx][idx]--;
        else {
            if(solver.problem.variables[idy]->isReduceAtLevel(solver.decisionLevel() - 1)) {
                (*a)[idx][idy]++;
                (*a)[idy][idx]++;
            } else {
                (*a)[idx][idy]--;
                (*a)[idy][idx]--;
            }
        }
    }
}


double HeuristicVarCRBS::score(Variable *x) {
    double sp = 0, sf = 0;
    for(int idy = 0; idy < solver.problem.nbVariables(); idy++) {
        if(solver.problem.variables[idy]->size() == 1)
            sp += (*a)[x->idx][idy];
        else
            sf += (*a)[x->idx][idy];
    }
    return (sp + theta * sf) / x->size();
}


Variable *HeuristicVarCRBS::select() {
    Variable *x    = solver.decisionVariables[0];
    double    best = score(x);
    for(int i = 1; i < solver.decisionVariables.size(); i++) {
        Variable *y = solver.decisionVariables[i];
        double    d = score(y);
        if(d > best) {
            best = d;
            x    = y;
        }
    }
    return x;
}
