#include "ShortSTR2.h"

#include "mtl/Matrix.h"
#include "solver/Solver.h"
#include "utils/Constants.h"
// TODO restore lastsize when backtracking

using namespace Cosoco;
#define UNKNOWN (-1)

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool ShortSTR2::isSatisfiedBy(vec<int> &tuple) {
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


bool ShortSTR2::isCorrectlyDefined() { return true; }


//----------------------------------------------
// Main filtering method
//----------------------------------------------

bool ShortSTR2::filter(Variable *dummy) {
    Ssup.clear();
    Sval.clear();
    int posLast = NOTINSCOPE;
    if(solver->decisionLevel() > 0) {
        Variable *last = solver->decisionVariableAtLevel(solver->decisionLevel());
        posLast        = toScopePosition(last->idx);
        if(posLast != NOTINSCOPE)
            Sval.add(posLast);
    }

    // Construct Sval
    for(int xpos = 0; xpos < scope.size(); xpos++) {
        if(xpos == posLast)
            continue;
        gacIdValues[xpos]->clear();
        Ssup.add(xpos);
        if(scope[xpos]->size() != lastSize[xpos]) {
            Sval.add(xpos);
            lastSize[xpos] = scope[xpos]->size();
        }
    }

    // Check tuple validity and construct Ssup
    for(int tupidx : reverse(validTuples)) {   // Reverse traversal because of deletion
        int *currentTuple = (*tuples)[tupidx];
        if(isValidTuple(currentTuple)) {
            for(int xpos : reverse(Ssup)) {   // Reverse traversal because of deletion
                if(currentTuple[xpos] == STAR)
                    Ssup.del(xpos);
                else {
                    gacIdValues[xpos]->add(currentTuple[xpos]);
                    if(gacIdValues[xpos]->size() == scope[xpos]->size())
                        Ssup.del(xpos);
                }
            }
        } else
            delTuple(tupidx, solver->decisionLevel());
    }
    if(validTuples.size() == 0)
        return false;

    // Delete bad values from tuples
    for(int xpos : Ssup) {
        if((gacIdValues[xpos]->size() == 0))
            return false;
        if(solver->changeDomain(scope[xpos], *(gacIdValues[xpos])) == false)
            return false;
        lastSize[xpos] = scope[xpos]->size();
    }
    return true;
}


//----------------------------------------------
// Tuples methods
//----------------------------------------------

bool ShortSTR2::isValidTuple(int *tupleIdvs) {
    for(int posx : Sval)
        if(tupleIdvs[posx] != STAR && scope[posx]->containsIdv(tupleIdvs[posx]) == false)
            return false;
    return true;
}


void ShortSTR2::delTuple(int position, int level) {
    if(validTuples.isLimitRecordedAtLevel(level) == false)
        validTuples.recordLimit(level);
    validTuples.del(position);
}

//----------------------------------------------
// Observers methods
//----------------------------------------------


void ShortSTR2::notifyDeleteDecision(Variable *x, int v, Solver &s) {
    if(validTuples.isLimitRecordedAtLevel(s.decisionLevel() + 1))
        validTuples.restoreLimit(s.decisionLevel() + 1);
    for(int i = 0; i < scope.size(); i++) lastSize[i] = UNKNOWN;
}


//----------------------------------------------
// Constructors and initialisation
//----------------------------------------------

ShortSTR2::ShortSTR2(Problem &p, std::string n, vec<Variable *> &vars, size_t max_n_tuples)
    : Extension(p, n, vars, max_n_tuples, true), Sval(vars.size()), Ssup(vars.size()), validTuples() {
    lastSize.growTo(vars.size(), UNKNOWN);
    type = "Extension";
}


ShortSTR2::ShortSTR2(Problem &p, std::string n, vec<Variable *> &vars, Matrix<int> *tuplesFromOtherConstraint)
    : Extension(p, n, vars, true, tuplesFromOtherConstraint), Sval(vars.size()), Ssup(vars.size()), validTuples() {
    lastSize.growTo(vars.size(), UNKNOWN);
    type = "Extension";
}


void ShortSTR2::delayedConstruction(int id) {
    Constraint::delayedConstruction(id);
    for(Variable *x : scope) gacIdValues.push(new SparseSet(x->domain.maxSize()));
    validTuples.setCapacity(tuples->nrows(), true);
}


void ShortSTR2::attachSolver(Solver *s) {
    Constraint::attachSolver(s);
    s->addObserverDeleteDecision(this);   // We need to restore validTuples.
}
