#include "Disjunctive.h"

#include <algorithm>

#include "solver/Solver.h"
using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool Disjunctive::isSatisfiedBy(vec<int> &tuple) { return tuple[0] + lx <= tuple[1] || tuple[1] + ly <= tuple[0]; }

//----------------------------------------------
// Filtering
//----------------------------------------------

bool Disjunctive::filterDomain(Variable *z, int lbValue, int ubValue) {
    if(lbValue > ubValue)
        return true;   // nothing to filter

    if(z->size() == 1) {
        int v = z->value();
        return !(lbValue <= v && v <= ubValue);
    }
    for(int idv : z->domain) {
        int v = z->domain.toVal(idv);
        if(v >= lbValue && v <= lbValue && solver->delVal(z, v) == false)
            return false;
    }
    return true;
}


bool Disjunctive::filter(Variable *xx) {
    if(solver->delValuesInRange(x, y->maximum() - lx + 1, y->minimum() + ly) == false)
        return false;
    if(solver->delValuesInRange(y, x->maximum() - ly + 1, x->minimum() + lx) == false)
        return false;
    return true;
    // return dx.removeValuesInRange(dy.lastValue() - kx + 1, dy.firstValue() + ky)
    //        && dy.removeValuesInRange(dx.lastValue() - ky + 1, dx.firstValue() + kx);
    // return ;
    /*
        while(true) {
            int minx = x->minimum(), maxx = x->maximum();
            int miny = y->minimum(), maxy = y->maximum();
            if(filterDomain(x, std::max(minx, std::min(maxx, maxy - lx) + 1), std::min(maxx, std::max(minx, miny + ly) - 1)) ==
       false) return false; if(filterDomain(y, std::max(miny, std::min(maxy, maxx - ly) + 1), std::min(maxy, std::max(miny, minx +
       lx) - 1)) == false) return false;

            if(minx == x->minimum() && maxx == x->maximum() && miny == y->minimum() && maxy == y->maximum())
                break;
        }
        */
    return true;
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

Disjunctive::Disjunctive(Problem &p, std::string n, Variable *xx, Variable *yy, int ll1, int ll2)
    : Binary(p, n, xx, yy), lx(ll1), ly(ll2) {
    type = "Disjunctive";
}
