
#include "GEUnary.h"

#include "LE.h"
#include "solver/Solver.h"

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool GEUnary::isSatisfiedBy(vec<int> &tuple) { return tuple[0] >= k; }

//----------------------------------------------
// Filtering
//----------------------------------------------

bool GEUnary::filter(Variable *x) {
    if(solver->delValuesLowerOrEqualThan(x, k - 1) == false)
        return false;
    solver->entail(this);
    return true;
}


//----------------------------------------------
// Objective constraint
//----------------------------------------------

void GEUnary::updateBound(long bound) { k = bound; }   // Update the current bound

long GEUnary::maxUpperBound() { return scope[0]->maximum(); }

long GEUnary::minLowerBound() { return scope[0]->minimum(); }


long GEUnary::computeScore(vec<int> &solution) { return solution[0]; }

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------
GEUnary::GEUnary(Problem &p, std::string n, Variable *xx, int kk) : Constraint(p, n), k(kk) {
    addToScope(xx);
    type = "X >= k";
}
