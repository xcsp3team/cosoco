#include "AC3rm.h"

#include "solver/Solver.h"

using namespace Cosoco;
using namespace std;

//----------------------------------------------------------
// check validity
//----------------------------------------------------------

bool AdapterAC3rm::isSatisfiedBy(vec<int> &tuple) { return constraint->isSatisfiedBy(tuple); }


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool AdapterAC3rm::filter(Variable *x) {
    if(isItTimeToStartFiltering() == false)
        return true;

    for(int posx = 0; posx < scope.size(); posx++) {
        // if (scope.size() > 1 && scope[posx] == x) continue; // this variable has already been filtered
        if(revise(scope[posx], posx) == false)
            return false;
    }
    return true;
}

bool AdapterAC3rm::isItTimeToStartFiltering() {
    unsigned long long nb = 1;
    // return false;
    for(Variable *x : scope) {
        assert(x->size() > 0);
        nb *= x->size();
        if(nb > maxSize)
            return false;
    }
    assert(nb > 0);
    return true;
}


bool AdapterAC3rm::revise(Variable *x, int posx) {
    for(int idv : x->domain) {
        vec<int> *currentResidue = residue(posx, idv);
        assert(currentResidue == nullptr || (*currentResidue)[posx] == idv);

        if(currentResidue == nullptr || constraint->isSatisfiedByOfIndexes(*currentResidue) == false)
            if(seekSupport(posx, idv) == false && solver->delIdv(x, idv) == false)
                return false;
    }
    return true;
}


bool AdapterAC3rm::seekSupport(int posx, int idv) {
    tupleIterator.setMinimalTuple(posx, idv);

    while(tupleIterator.hasNextTuple()) {
        vec<int> &tmp = *(tupleIterator.nextTuple());
        if(constraint->isSatisfiedByOfIndexes(tmp)) {
            storeResidues(tmp);
            return true;
        }
    }
    return false;
}


vec<int> *AdapterAC3rm::residue(int posx, int idv) {
    if(_residues[posx][idv].size() == 0)
        return nullptr;
    vec<int> &tmp = _residues[posx][idv];

    for(int i = 0; i < tmp.size(); i++)
        if(scope[i]->containsIdv(tmp[i]) == false)
            return nullptr;

    return &(_residues[posx][idv]);
}


void AdapterAC3rm::storeResidues(vec<int> &tmp) {
    for(int posx = 0; posx < tmp.size(); posx++) tmp.copyTo(_residues[posx][tmp[posx]]);
}


//----------------------------------------------
// Constructor and initialisation methods
//----------------------------------------------


AdapterAC3rm::AdapterAC3rm(Constraint *c)
    : Constraint(c->problem, "AC3rm on " + c->name, c->scope), constraint(c), tupleIterator(c->scope) {
    type   = c->type;
    int ar = scope.size();
    _residues.growTo(ar);
    for(int i = 0; i < ar; i++) {
        _residues[i].growTo(scope[i]->size());
    }
}


bool AdapterAC3rm::isCorrectlyDefined() { return constraint->isCorrectlyDefined(); }

void AdapterAC3rm::delayedConstruction(int id) { constraint->makeDelayedConstruction(id); }

State AdapterAC3rm::status() { return constraint->status(); }


void AdapterAC3rm::reinitialize() { constraint->reinitialize(); }


// in general constraints does not care => no abstract.
void AdapterAC3rm::attachSolver(Solver *s) {
    Constraint::attachSolver(s);
    constraint->attachSolver(s);
    maxSize = solver->options.intOptions["limitac3card"].value;
}

void AdapterAC3rm::display(bool d) { constraint->display(d); }
//---------------------------------------------------------------------------------
// Objective functions
// The attached constraint must be an ObjectiveConstraint
//---------------------------------------------------------------------------------


void AdapterAC3rm::updateBound(long bound) {
    auto *objective = dynamic_cast<ObjectiveConstraint *>(constraint);
    return objective->updateBound(bound);
}


long AdapterAC3rm::maxUpperBound() {
    auto *objective = dynamic_cast<ObjectiveConstraint *>(constraint);
    return objective->maxUpperBound();
}


long AdapterAC3rm::minLowerBound() {
    auto *objective = dynamic_cast<ObjectiveConstraint *>(constraint);
    return objective->minLowerBound();
}


long AdapterAC3rm::computeScore(vec<int> &solution) {
    auto *objective = dynamic_cast<ObjectiveConstraint *>(constraint);
    return objective->computeScore(solution);
}
