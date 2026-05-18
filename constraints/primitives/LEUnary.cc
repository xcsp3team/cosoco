
#include "LEUnary.h"

#include "solver/Solver.h"

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool LEUnary::isSatisfiedBy(vec<int> &tuple) { return tuple[0] <= k; }

//----------------------------------------------
// Filtering
//----------------------------------------------

bool LEUnary::filter(Variable *x) {
    if(solver->delValuesGreaterOrEqualThan(x, k + 1) == false)
        return false;
    solver->entail(this);
    return true;
}


//----------------------------------------------
// Objective constraint
//----------------------------------------------

void LEUnary::updateBound(long bound) { k = bound; }   // Update the current bound

long LEUnary::maxUpperBound() { return scope[0]->maximum(); }

long LEUnary::minLowerBound() { return scope[0]->minimum(); }


long LEUnary::computeScore(vec<int> &solution) { return solution[0]; }

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------
LEUnary::LEUnary(Problem &p, std::string n, Variable *xx, int kk) : Constraint(p, n), k(kk) {
    addToScope(xx);
    type = "X <= k";
}
