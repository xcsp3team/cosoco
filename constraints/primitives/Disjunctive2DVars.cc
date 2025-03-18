#include <algorithm>

#include "Disjunctive.h"
#include "solver/Solver.h"
using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool Disjunctive2DVar::isSatisfiedBy(vec<int> &t) {
    return ((t[8] == 0) && (t[0] + t[4] <= t[1])) || ((t[8] == 1) && (t[1] + t[5] <= t[0])) ||
           ((t[8] == 2) && (t[2] + t[6] <= t[3])) || ((t[8] == 3) && (t[3] + t[7] <= t[2]));
}

//----------------------------------------------
// Filtering
//----------------------------------------------


bool Disjunctive2DVar::filter(Variable *xx) {
    int  minx1 = x1->minimum() + w1->minimum(), minx2 = x2->minimum() + w2->minimum();
    int  miny1 = y1->minimum() + h1->minimum(), miny2 = y2->minimum() + h2->minimum();
    bool bx1 = minx1 <= x2->maximum(), bx2 = minx2 <= x1->maximum();
    bool by1 = miny1 <= y2->maximum(), by2 = miny2 <= y1->maximum();

    if(!bx2 || (x1->maximum() + w1->maximum() <= x2->minimum()))   // !bx2 or x1 + w1 <= x2 => z != 1
        if(solver->delVal(z, 1) == false)
            return false;
    if(!bx1 || (x2->maximum() + w2->maximum() <= x1->minimum()))   // !bx1 or x2 + w2 <= x1 => z != 0
        if(solver->delVal(z, 0) == false)
            return false;
    if(!by2 || (y1->maximum() + h1->maximum() <= y2->minimum()))   // !by2 or y1 + h1 <= y2 => z != 3
        if(solver->delVal(z, 3) == false)
            return false;
    if(!by1 || (y2->maximum() + h2->maximum() <= y1->minimum()))   // !by1 or y2 + h2 <= y1 => z != 2
        if(solver->delVal(z, 2) == false)
            return false;

    if(z->size() == 1) {
        if(z->value() == 0)
            return solver->enforceLE(x1, w1, x2);   // z = 0 => x1 + w1 <= x2
        if(z->value() == 1)
            return solver->enforceLE(x2, w2, x1);   // z = 1 => x2 + w2 <= x1
        if(z->value() == 2)
            return solver->enforceLE(y1, h1, y2);   // z = 2 => y1 + h1 <= y2
        if(z->value() == 3)
            return solver->enforceLE(y2, h2, y1);   // z = 3 => y2 + h2 <= y1
    }

    bool bx = bx1 || bx2, by = by1 || by2;
    if(bx && by)
        return true;
    if(!bx && !by)
        return false;
    assert(z->size() == 2);   // because otherwise z would have been reduced earlier
    if(bx) {
        assert(bx1 && bx2);
        if(solver->delValuesInRange(x1, x2->maximum() - w1->minimum() + 1, minx2) == false)
            return false;
        if(solver->delValuesInRange(x2, x1->maximum() - w2->minimum() + 1, minx1) == false)
            return false;
        // if (!bx1)
        // if (w2->removeValuesGT(x1->maximum() - x2->minimum()) == false)
        // return false;
        // if (!bx2)
        // if (w1->removeValuesGT(x2->maximum() - x1->minimum()) == false)
        // return false;
        return true;
    } else {
        assert(by1 && by2);
        if(solver->delValuesInRange(y1, y2->maximum() - h1->minimum() + 1, miny2) == false)
            return false;
        if(solver->delValuesInRange(y2, y1->maximum() - h2->minimum() + 1, miny1) == false)
            return false;
        return true;
    }
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

Disjunctive2DVar::Disjunctive2DVar(Problem &p, std::string n, Variable *xx1, Variable *xx2, Variable *yy1, Variable *yy2,
                                   Variable *ww1, Variable *ww2, Variable *hh1, Variable *hh2, Variable *zz)
    : GlobalConstraint(p, n, "Disjunctive2DVar", Constraint::createScopeVec(xx1, xx2, yy1, yy2, ww1, ww2, hh1, hh2, zz)) {
    x1 = xx1;
    x2 = xx2;
    y1 = yy1;
    y2 = yy2;
    z  = zz;
    w1 = ww1;
    w2 = ww2;
    h1 = hh1;
    h2 = hh2;
}
