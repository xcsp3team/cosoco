//
// Created by audemard on 09/04/25.
//

#include "STR0.h"

#include <Solver.h>

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool STR0::isSatisfiedBy(vec<int> &tuple) {
    for(unsigned int i = 0; i < tuples->nrows(); i++) {
        int nb = 0;
        for(int j = 0; j < scope.size(); j++) {
            if((*tuples)[i][j] != STAR && tuple[j] != scope[j]->domain.toVal((*tuples)[i][j]))
                break;
            nb++;
        }
        if(nb == scope.size())
            return true;
    }
    return false;
}

bool STR0::isCorrectlyDefined() { return true; }


//----------------------------------------------
// Main filtering method
//----------------------------------------------
void STR0::beforeFiltering() {
    sSupSize = 0;
    for(const int idx : unassignedVariablesIdx) {
        cnts[idx]        = scope[idx]->size();
        sSup[sSupSize++] = idx;
        ac[idx].fill(false);
    }
}


bool STR0::isValidTuple(int *t) {
    universal = true;
    for(int idx = 0; idx < scope.size(); idx++) {
        if(t[idx] == STAR)
            continue;
        if(scope[idx]->containsIdv(t[idx]) == false)
            return false;
        if(scope[idx]->size() > 1)
            universal = false;
    }
    return true;
}

bool STR0::updateDomains() {
    for(int i = sSupSize - 1; i >= 0; i--) {
        int x         = sSup[i];
        int nRemovals = cnts[x];
        assert(nRemovals > 0);
        for(int tmp = 0, idv = scope[x]->domain[0]; tmp < nRemovals; idv++) {
            if(ac[x][idv] == false) {
                if(solver->delIdv(scope[x], idv) == false)
                    return false;
                tmp++;
            }
        }
    }
    return true;
}


bool STR0::filter(Variable *dummy) {
    beforeFiltering();
    for(int i : set) {
        int *tuple = (*tuples)[i];
        if(isValidTuple(tuple)) {
            if(universal) {
                solver->entail(this);
                return true;
            }
            for(int j = sSupSize - 1; j >= 0; j--) {
                int x = sSup[j];
                int a = tuple[x];
                if(a == STAR) {
                    cnts[x] = 0;
                    sSup[j] = sSup[--sSupSize];
                } else if(!ac[x][a]) {
                    ac[x][a] = true;
                    if(--cnts[x] == 0)
                        sSup[j] = sSup[--sSupSize];
                }
            }
        } else
            set.del(i);
    }
    return updateDomains();
}


//----------------------------------------------
// Observers methods
//----------------------------------------------


void STR0::notifyDeleteDecision(Variable *x, int v, Solver &s) { set.fill(); }


//----------------------------------------------
// Constructors and initialisation
//----------------------------------------------

STR0::STR0(Problem &p, std::string n, vec<Variable *> &vars, size_t max_n_tuples) : Extension(p, n, vars, max_n_tuples, true) {
    type = "STR0";
}


STR0::STR0(Problem &p, std::string n, vec<Variable *> &vars, Matrix<int> *tuplesFromOtherConstraint)
    : Extension(p, n, vars, true, tuplesFromOtherConstraint) {
    type = "STR0";
}


void STR0::delayedConstruction(int id) {
    Constraint::delayedConstruction(id);
    set.setCapacity(nbTuples(), true);
    ac.growTo(scope.size());
    for(int i = 0; i < scope.size(); i++) ac[i].growTo(scope[i]->domain.maxSize());
    cnts = new int[scope.size()];
    sSup = new int[scope.size()];
}


void STR0::attachSolver(Solver *s) {
    Constraint::attachSolver(s);
    s->addObserverDeleteDecision(this);   // We need to restore validTuples.
}
