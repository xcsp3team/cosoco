#include "STRNeg.h"

#include "solver/Solver.h"
#include "utils/Utils.h"

using namespace Cosoco;


//----------------------------------------------
// check validity and correct definition
//----------------------------------------------

bool STRNeg::isSatisfiedBy(vec<int> &tuple) {
    for(auto &tmp : tuples) {
        int nb = 0;
        for(int j = 0; j < tmp.size(); j++) {
            if(tmp[j] != scope[j]->domain.toIdv(tuple[j]))
                break;
            nb++;
        }
        if(nb == scope.size())
            return false;
    }
    return true;
}


bool STRNeg::isCorrectlyDefined() {
    for(vec<int> &v : tuples)
        if(v.size() != scope.size())
            throw std::logic_error("Constraint " + std::to_string(idc) + ": STRNeg has a tuple with bad size");
    return true;
}

//----------------------------------------------------------
// Filtering
//----------------------------------------------------------


void STRNeg::initializeStructuresBeforeFiltering() {
    long nbTotalValidTuples = 1;
    for(Variable *x : scope) nbTotalValidTuples *= x->size();

    assert(nbTotalValidTuples > 0);

    variablesToCheck.clear();

    for(int posx = 0; posx < scope.size(); posx++) {
        Variable *x  = scope[posx];
        long      nb = nbTotalValidTuples / x->size();
        assert(nbTotalValidTuples % x->size() == 0);
        if(validTuples.size() >= nb && validTuples.size() >= maxNbConflicts[posx]) {
            variablesToCheck.add(posx);
            maxNbConflicts[posx] = 0;
            nbValidTuples[posx]  = nb;
            for(int i = 0; i < nbConflicts[posx].size(); i++) nbConflicts[posx][i] = 0;
        }
    }
}


bool STRNeg::filter(Variable *dummy) {
    if(tuples.size() == 0)
        return true;

    for(int posx = 0; posx < scope.size(); posx++)
        for(int i = 0; i < nbConflicts[posx].size(); i++) nbConflicts[posx][i] = 0;


    for(int i = validTuples.size() - 1; i >= 0; i--) {   // Reverse traversal because of deletion
        vec<int> &currentTuple = tuples[validTuples[i]];
        if(isValidTuple(currentTuple)) {
            for(int posx : unassignedVariablesIdx) {
                int idv = currentTuple[posx];
                nbConflicts[posx][idv]++;
            }
        } else
            delTuple(validTuples[i], solver->decisionLevel());
    }

    long nbTotalValidTuples = 1;
    for(Variable *x : scope) nbTotalValidTuples *= x->size();

    for(int posx : unassignedVariablesIdx) {
        Variable *x                     = scope[posx];
        long      nbValidTuplesOfValues = nbTotalValidTuples / x->size();
        for(int idv : reverse(x->domain))
            if(nbConflicts[posx][idv] == nbValidTuplesOfValues &&
               solver->delIdv(scope[posx], idv) == false)   // All possible values are in valid conflict tuples
                return false;
    }
    return true;


    /*   BUGGY VERSION SEE AIM !!!
     * initializeStructuresBeforeFiltering();
     * for (int i = validTuples.size() - 1 ; i >= 0 ; i--) { // Reverse traversal because of deletion
            vec<int> &currentTuple = tuples[validTuples[i]];
            if (isValidTuple(currentTuple)) {
                for (int j = variablesToCheck.size() - 1 ; j >= 0 ; j--) { //Reverse traversal because od deletion
                    int posx = variablesToCheck[j];
                    int idv = currentTuple[posx];
                    nbConflicts[posx][idv]++;
                    maxNbConflicts[posx] = max(maxNbConflicts[posx], nbConflicts[posx][idv]);
                    if (nbConflicts[posx][idv] == nbValidTuples[posx]) { // All possible values are in valid conflict tuples
                        std::cout << idv << std::endl;
                        std::cout << nbConflicts[posx][idv] << " " << nbValidTuples[posx] << std::endl;
                        exit(1);
                            if (solver->delIdv(scope[posx], idv) == false)
                            return false;
                    } else {
                        if (maxNbConflicts[posx] + i < nbValidTuples[posx]) // We can not reach the previous condition, variable
     is ok variablesToCheck.del(j);

                    }
                }
            } else
                delTuple(validTuples[i], solver->decisionLevel());
        }
        return true;
        */
}


bool STRNeg::isValidTuple(vec<int> &tuple) {
    // for(int posx : unassignedVariablesIdx)
    for(int posx = 0; posx < scope.size(); posx++)
        if(scope[posx]->containsIdv(tuple[posx]) == false)
            return false;

    /*if(s.decisionLevel()>0) { //We also need to check the last decision variable.
        Variable *last = s.decisionVariableAtLevel(s.decisionLevel());
        return last->domain.containsIdv(tuple[idxToScopePosition[last->idx]]);
    }*/
    return true;
}


//----------------------------------------------
// Constructor and initialisation methods
//----------------------------------------------


STRNeg::STRNeg(Problem &p, std::string n, vec<Variable *> &vars) : Extension(p, n, vars, false) { type = "Extension"; }


STRNeg::STRNeg(Problem &p, std::string n, vec<Variable *> &vars, vec<vec<int> > &tuplesFromOtherConstraint)
    : Extension(p, n, vars, false, tuplesFromOtherConstraint) {
    type = "Extension";
}


void STRNeg::delayedConstruction(int id) {
    Constraint::delayedConstruction(id);

    if(tuples.size() == 0) // the constrain is always satisfied. Can occcur: see StillLife-wastage-8-8.xml
        return;


    validTuples.setCapacity(tuples.size(), true);

    nbValidTuples.growTo(scope.size());
    variablesToCheck.setCapacity(scope.size(), false);
    nbConflicts.growTo(scope.size());
    for(int i = 0; i < scope.size(); i++) nbConflicts[i].growTo(scope[i]->size());

    maxNbConflicts.growTo(scope.size(), 0);
}


//----------------------------------------------
// Observers methods
//----------------------------------------------


void STRNeg::notifyDeleteDecision(Variable *x, int v, Solver &s) {
    if(validTuples.isLimitRecordedAtLevel(s.decisionLevel() + 1))
        validTuples.restoreLimit(s.decisionLevel() + 1);
}


void STRNeg::delTuple(int position, int level) {
    if(validTuples.isLimitRecordedAtLevel(level) == false)
        validTuples.recordLimit(level);
    validTuples.del(position);
}


void STRNeg::attachSolver(Solver *s) {
    Constraint::attachSolver(s);
    s->addObserverDeleteDecision(this);   // We need to restore tuples.
}
