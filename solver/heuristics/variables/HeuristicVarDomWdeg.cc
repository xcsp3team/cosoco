#include "HeuristicVarDomWdeg.h"

#include "Options.h"

using namespace Cosoco;


HeuristicVarDomWdeg::HeuristicVarDomWdeg(Solver &s) : HeuristicVar(s) {
    s.addObserverConflict(this);
    s.addObserverDeleteDecision(this);
    s.addObserverNewDecision(this);
    mode = V2021;
    constraintsWeights.growTo(s.problem.nbConstraints());
    for(int i = 0; i < constraintsWeights.size(); i++)
        constraintsWeights[i].growTo(solver.problem.constraints[i]->scope.size(), 0);
    variablesWeights.growTo(s.problem.nbVariables(), 1);
}


Variable *HeuristicVarDomWdeg::select() {
    if(solver.warmStart && solver.conflicts == 0 && solver.nbSolutions == 0) {
        for(int i = 0; i < solver.decisionVariables.size(); i++)
            if(solver.decisionVariables[i]->_name.rfind("__av", 0) != 0)
                return solver.decisionVariables[i];
    }

    if(solver.warmStart == false && solver.statistics[restarts] < 1 && solver.nbSolutions == 0) {
        Constraint *c = solver.problem.constraints.last();
        for(Variable *x : c->scope)
            if(x->size() > 1)
                return x;
    }


    if(lazy && secondBest != nullptr && solver.unassignedVariables.contains(secondBest)) {
        Variable *tmp = secondBest;
        secondBest    = nullptr;
        return tmp;
    }


    Variable *x    = solver.decisionVariables[0];
    double    wdeg = variablesWeights[x->idx];
    if(mode == V2004 || mode == ABSCON) {
        double bestV = static_cast<double>(x->size()) / wdeg;

        for(int i = 1; i < solver.decisionVariables.size(); i++) {
            Variable *y     = solver.decisionVariables[i];
            double    ywdeg = variablesWeights[y->idx];
            if(static_cast<double>(y->size()) / ywdeg < bestV) {
                bestV      = static_cast<double>(y->size()) / ywdeg;
                secondBest = x;
                x          = y;
            } else {
                if(static_cast<double>(y->size()) / ywdeg == bestV)
                    secondBest = y;
            }
        }
        assert(x != nullptr);
        return x;
    }
    // MODE NEWWDEG

    if(mode == NEWWDEG) {
        double bestV = wdeg;

        for(int i = 1; i < solver.decisionVariables.size(); i++) {
            Variable *y     = solver.decisionVariables[i];
            double    ywdeg = variablesWeights[y->idx];
            if(ywdeg > bestV) {
                bestV      = ywdeg;
                secondBest = nullptr;
                x          = y;
            } else {
                if(ywdeg == bestV)
                    secondBest = y;
            }
        }
        return x;
    }


    double bestV = wdeg;

    for(int i = 1; i < solver.decisionVariables.size(); i++) {
        Variable *y     = solver.decisionVariables[i];
        double    ywdeg = variablesWeights[y->idx];

        if(ywdeg > bestV || (ywdeg == bestV && y->size() < x->size())) {
            bestV      = ywdeg;
            secondBest = nullptr;
            x          = y;
        } else {
            if(ywdeg == bestV)
                secondBest = y;
        }
    }

    assert(x != nullptr);
    return x;
}


void HeuristicVarDomWdeg::notifyConflict(Constraint *c, int level) {
    secondBest = nullptr;
    if(freezed)
        return;
    int notThisposition = NOTINSCOPE;
    if(c->unassignedVariablesIdx.size() == 1)
        notThisposition = c->unassignedVariablesIdx[0];   // c->toScopePosition(c->unassignedVariablesIdx[0]);


    if(mode == V2004) {
        for(int i = 0; i < c->scope.size(); i++) {
            if(i == notThisposition)
                continue;

            constraintsWeights[c->idc][i]++;
            variablesWeights[c->scope[i]->idx]++;
        }
    } else {
        for(int i = 0; i < c->unassignedVariablesIdx.size(); i++) {
            int posx = c->unassignedVariablesIdx[i];
            if(mode == ABSCON) {
                constraintsWeights[c->idc][posx]++;
                variablesWeights[i]++;
            } else {   // NEWWDEG
                constraintsWeights[c->idc][posx] +=
                    1 / (c->unassignedVariablesIdx.size() * (c->scope[posx]->size() == 0 ? 0.5 : c->scope[posx]->size()));
                variablesWeights[c->scope[i]->idx] +=
                    1 / (c->unassignedVariablesIdx.size() * (c->scope[posx]->size() == 0 ? 0.5 : c->scope[posx]->size()));
            }
        }
    }
}


void HeuristicVarDomWdeg::notifyNewDecision(Variable *x, Solver &s) {
    if(freezed)
        return;
    for(auto &pair : x->constraints) {
        Constraint *c    = pair.first;
        int         posx = c->unassignedVariablesIdx[0];
        if(c->unassignedVariablesIdx.size() == 1)
            variablesWeights[c->scope[posx]->idx] -= constraintsWeights[c->idc][posx];
    }
}


void HeuristicVarDomWdeg::notifyDeleteDecision(Variable *x, int v, Solver &s, bool isFull) {
    if(freezed)
        return;
    for(auto &pair : x->constraints) {
        Constraint *c    = pair.first;
        int         posx = c->unassignedVariablesIdx[0];

        if(c->unassignedVariablesIdx.size() == 2)
            variablesWeights[c->scope[posx]->idx] += constraintsWeights[c->idc][posx];
    }
}


void HeuristicVarDomWdeg::notifyFullBacktrack() {
    secondBest = nullptr;
    if(freezed)
        return;
    if(options::boolOptions["rw"].value == false)
        return;

    if(solver.statistics[restarts] > 0 && ((solver.statistics[restarts] + 1) - solver.lastSolutionRun) % 30 == 0) {
        printf("erer\n");
        for(int i = 0; i < constraintsWeights.size(); i++) constraintsWeights[i].fill(0);
        for(int i = 0; i < variablesWeights.size(); i++) variablesWeights[i] = 1;
    }
}
