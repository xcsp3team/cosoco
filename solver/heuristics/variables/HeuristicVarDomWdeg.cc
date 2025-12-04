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
    variablesWeights.growTo(s.problem.nbVariables(), 0);
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


    if(options::boolOptions["lazyvar"].value && secondBest != nullptr && solver.unassignedVariables.contains(secondBest)) {
        Variable *tmp = secondBest;
        secondBest    = nullptr;
        return tmp;
    }


    Variable *x = solver.decisionVariables[0];
    if(mode == V2004 || mode == ABSCON) {
        double bestV = ((double)x->size()) / x->wdeg;

        for(int i = 1; i < solver.decisionVariables.size(); i++) {
            Variable *y = solver.decisionVariables[i];
            if(((double)y->size()) / y->wdeg < bestV) {
                bestV      = ((double)y->size()) / y->wdeg;
                secondBest = x;
                x          = y;
            } else {
                if(((double)y->size()) / y->wdeg == bestV)
                    secondBest = y;
            }
        }
        assert(x != nullptr);
        return x;
    }
    // MODE NEWWDEG

    if(mode == NEWWDEG) {
        double bestV = ((double)x->wdeg);

        for(int i = 1; i < solver.decisionVariables.size(); i++) {
            Variable *y = solver.decisionVariables[i];
            if(y->wdeg > bestV) {
                bestV      = y->wdeg;
                secondBest = nullptr;
                x          = y;
            } else {
                if(y->wdeg == bestV)
                    secondBest = y;
            }
        }
        return x;
    }


    double bestV = ((double)x->wdeg);

    for(int i = 1; i < solver.decisionVariables.size(); i++) {
        Variable *y = solver.decisionVariables[i];
        if(y->wdeg > bestV || (y->wdeg == bestV && y->size() < x->size())) {
            bestV      = y->wdeg;
            secondBest = nullptr;
            x          = y;
        } else {
            if(y->wdeg == bestV)
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
            c->wdeg[i]++;
            c->scope[i]->wdeg++;
        }
    } else {
        for(int i = 0; i < c->unassignedVariablesIdx.size(); i++) {
            int posx = c->unassignedVariablesIdx[i];
            if(mode == ABSCON) {
                c->wdeg[posx]++;
                c->scope[posx]->wdeg++;
            } else {   // NEWWDEG
                c->wdeg[posx] +=
                    1 / (c->unassignedVariablesIdx.size() * (c->scope[posx]->size() == 0 ? 0.5 : c->scope[posx]->size()));
                c->scope[posx]->wdeg +=
                    1 / (c->unassignedVariablesIdx.size() * (c->scope[posx]->size() == 0 ? 0.5 : c->scope[posx]->size()));
            }
        }
    }
}


void HeuristicVarDomWdeg::notifyNewDecision(Variable *x, Solver &s) {
    if(freezed)
        return;
    for(Constraint *c : x->constraints) {
        if(c->unassignedVariablesIdx.size() == 1)
            c->scope[c->unassignedVariablesIdx[0]]->wdeg -= c->wdeg[c->unassignedVariablesIdx[0]];
    }
}


void HeuristicVarDomWdeg::notifyDeleteDecision(Variable *x, int v, Solver &s) {
    if(freezed)
        return;
    for(Constraint *c : x->constraints) {
        if(c->unassignedVariablesIdx.size() == 2)
            c->scope[c->unassignedVariablesIdx[0]]->wdeg += c->wdeg[c->unassignedVariablesIdx[0]];
    }
}


void HeuristicVarDomWdeg::notifyFullBacktrack() {
    secondBest = nullptr;
    if(freezed)
        return;
    if(options::boolOptions["rw"].value == false)
        return;

    if(solver.statistics[GlobalStats::restarts] > 0 &&
       ((solver.statistics[GlobalStats::restarts] + 1) - solver.lastSolutionRun) % 30 == 0) {
        printf("erer\n");
        for(Constraint *c : solver.problem.constraints) c->wdeg.fill(0);
        for(Variable *x : solver.problem.variables) x->wdeg = 0;
    }
}

bool HeuristicVarDomWdeg::start() {
    freezed = false;
    for(int i = 0; i < variablesWeights.size(); i++) variablesWeights[i] = solver.problem.variables[i]->wdeg;
    for(int i = 0; i < constraintsWeights.size(); i++) solver.problem.constraints[i]->wdeg.copyTo(constraintsWeights[i]);
    return false;
}


bool HeuristicVarDomWdeg::stop() {
    freezed = true;
    for(int i = 0; i < variablesWeights.size(); i++) solver.problem.variables[i]->wdeg = variablesWeights[i];
    for(int i = 0; i < constraintsWeights.size(); i++) constraintsWeights[i].copyTo(solver.problem.constraints[i]->wdeg);
    return false;
}