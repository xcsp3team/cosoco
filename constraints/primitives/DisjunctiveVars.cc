//
// Created by audemard on 28/01/23.
//

#include "DisjunctiveVars.h"

#include "solver/Solver.h"
using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool DisjunctiveVars::isSatisfiedBy(vec<int> &t) { return t[0] + t[2] <= t[1] || t[1] + t[3] <= t[0]; }

//----------------------------------------------
// Filtering
//----------------------------------------------


bool DisjunctiveVars::filter(Variable *xx) {
    int  min1 = x1->minimum() + w1->minimum(), min2 = x2->minimum() + w2->minimum();
    int  max1 = x2->maximum() - w1->minimum(), max2 = x1->maximum() - w2->minimum();
    bool b1 = min1 <= x2->maximum(), b2 = min2 <= x1->maximum();
    if(!b1 && !b2)
        return false;
    if(!b2)   // we enforce the first part
        return solver->delValuesLowerOrEqualThan(x2, min1 - 1) && solver->delValuesGreaterOrEqualThan(x1, max1 + 1) &&
               solver->delValuesGreaterOrEqualThan(w1, x2->maximum() - x1->minimum() + 1);
    //    if (!b2) // we enforce the first part
    //        return dx2.removeValuesLT(min1) && dx1.removeValuesGT(max1) && dw1.removeValuesGT(dx2.lastValue() -
    //        dx1.firstValue());
    ///    if (!b1) // we enforce the second part
    //       return dx1.removeValuesLT(min2) && dx2.removeValuesGT(max2) && dw2.removeValuesGT(dx1.lastValue() -
    //       dx2.firstValue());

    if(!b1)   // we enforce the second part
        return solver->delValuesLowerOrEqualThan(x1, min2 - 1) && solver->delValuesGreaterOrEqualThan(x2, max2 + 1) &&
               solver->delValuesGreaterOrEqualThan(w2, x1->maximum() - x2->minimum() + 1);
    return solver->delValuesInRange(x1, max1 + 1, min2 - 1) && solver->delValuesInRange(x2, max2 + 1, min1 - 1);
    //        dx1.removeValuesInRange(max1 + 1, min2) && dx2.removeValuesInRange(max2 + 1, min1);
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

DisjunctiveVars::DisjunctiveVars(Problem &p, std::string n, Variable *xx1, Variable *xx2, Variable *ww1, Variable *ww2)
    : GlobalConstraint(p, n, "DisjunctiveVars", Constraint::createScopeVec(xx1, xx2, ww1, ww2)) {
    x1 = xx1;
    x2 = xx2;
    w1 = ww1;
    w2 = ww2;
}