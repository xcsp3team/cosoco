#include "NoOverlap2D.h"

#include "solver/Solver.h"
using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool NoOverlap2D::isSatisfiedBy(vec<int> &t) {
    return true;   // TODO
}

//----------------------------------------------
// Filtering
//----------------------------------------------


bool NoOverlap2D::filter(Variable *xx) {
    bool hasFiltered = true;
    prop(xx->idx);

    while(hasFiltered) {
        hasFiltered = false;
        boxesToCompute.clear();
        for(int i = 0; i < n; i++) {
            boxesToCompute.push(i);
            for(int j = i + 1; j < n; j++) {
                if(mayOverlap(i, j)) {
                    overlappingBoxes.addEdge(i, j);
                    if(boxInstantiated(i) && boxInstantiated(j)) {
                        return false;
                    }
                } else {
                    overlappingBoxes.removeEdge(i, j);
                }
            }
        }
        pruneList.clear();
        for(int i : boxesToCompute) {
            if(energyCheck(i) == false)
                return false;
            int tmp = prune(i);
            if(tmp == CONFLICT)
                return false;
            hasFiltered |= tmp;
        }
        boxesToCompute.clear();
        for(int k : pruneList) {
            prop(k);
        }
    }
    return true;
}

bool NoOverlap2D::boxInstantiated(int i) {
    return scope[i]->isAssigned() && scope[i + n]->isAssigned() && scope[i + 2 * n]->isAssigned() &&
           scope[i + 3 * n]->isAssigned();
}

bool NoOverlap2D::mayOverlap(int i, int j) { return isNotDisjoint(i, j, true) && isNotDisjoint(i, j, false); }


bool NoOverlap2D::isNotDisjoint(int i, int j, bool horizontal) {
    int off = (horizontal) ? 0 : n;
    return (scope[i + off]->minimum() < scope[j + off]->maximum() + scope[j + off + 2 * n]->maximum()) &&
           (scope[j + off]->minimum() < scope[i + off]->maximum() + scope[i + off + 2 * n]->maximum());
}

bool NoOverlap2D::doOverlap(int i, int j, bool hori) {
    int offSet = hori ? 0 : n;
    int s_i    = scope[i + offSet]->maximum();
    int e_i    = scope[i + offSet]->minimum() + scope[i + 2 * n + offSet]->minimum();
    int s_j    = scope[j + offSet]->maximum();
    int e_j    = scope[j + offSet]->minimum() + scope[j + 2 * n + offSet]->minimum();
    return (s_i < e_i && e_j > s_i && s_j < e_i) || (s_j < e_j && e_i > s_j && s_i < e_j);
}

int NoOverlap2D::prune(int j) {
    bool hasFiltered = USELESS;

    for(int i : overlappingBoxes.edges[j]) {
        if(doOverlap(i, j, true)) {
            int tmp = doFiltering(i, j, false);
            if(tmp == CONFLICT)
                return CONFLICT;
            if(tmp == PROP)
                hasFiltered = PROP;
        }
        if(doOverlap(i, j, false)) {
            int tmp = doFiltering(i, j, true);
            if(tmp == CONFLICT)
                return CONFLICT;
            if(tmp == PROP)
                hasFiltered = PROP;
        }
    }
    return hasFiltered;
}


int NoOverlap2D::doFiltering(int i, int j, bool hori) {
    int hasFiltered = USELESS;
    int offSet      = hori ? 0 : n;
    int s_i         = scope[i + offSet]->maximum();                                          // latest start of i
    int e_i         = scope[i + offSet]->minimum() + scope[i + 2 * n + offSet]->minimum();   // earliest end of i
    int s_j         = scope[j + offSet]->maximum();                                          // latest start of j
    int e_j         = scope[j + offSet]->minimum() + scope[j + 2 * n + offSet]->minimum();   // earliest end of j
    if(s_i < e_i || s_j < e_j) {   // if at least one mandatory part is not empty
        if(e_j > s_i) {            // if j ends after i starts
            // then update the start of j to the end of i
            int szBefore = scope[j + offSet]->size();
            if(solver->delValuesLowerOrEqualThan(scope[j + offSet], e_i - 1) == false)
                return CONFLICT;
            if(scope[j + offSet]->size() != szBefore) {
                if(pruneList.contains(j) == false) {
                    pruneList.push(j);
                }
                hasFiltered = PROP;
            }
            // and update the start of i to the end of j
            szBefore = scope[i + offSet]->size();
            if(solver->delValuesGreaterOrEqualThan(scope[i + offSet], s_j - scope[i + 2 * n + offSet]->minimum() + 1) == false)
                return CONFLICT;
            bool filtPrun1 = scope[i + offSet]->size() != szBefore;

            szBefore = scope[i + offSet + 2 * n]->size();
            if(solver->delValuesGreaterOrEqualThan(scope[i + offSet + 2 * n], s_j - scope[i + offSet]->minimum() + 1) == false)
                return CONFLICT;
            bool filtPrun2 = scope[i + offSet + 2 * n]->size() != szBefore;
            if(filtPrun1 || filtPrun2) {
                if(!pruneList.contains(i)) {
                    pruneList.push(i);
                }
                hasFiltered = PROP;
            }
        }
        if(s_j < e_i) {   // if j starts before i ends
            // then update the start of i to the end of j
            int szBefore = scope[i + offSet]->size();
            if(solver->delValuesLowerOrEqualThan(scope[i + offSet], e_j - 1) == false)
                return CONFLICT;
            if(szBefore != scope[i + offSet]->size()) {
                if(!pruneList.contains(i)) {
                    pruneList.push(i);
                }
                hasFiltered = PROP;
            }

            // and update the start of j to the end of i
            szBefore = scope[j + offSet]->size();
            if(solver->delValuesGreaterOrEqualThan(scope[j + offSet], s_i - scope[j + 2 * n + offSet]->minimum() + 1) == false)
                return CONFLICT;
            bool filtPrun1 = szBefore != scope[j + offSet]->size();
            // and update the size of j to the space between the earliest start of i and the earliest start of j
            szBefore = scope[j + offSet + 2 * n]->size();
            if(solver->delValuesGreaterOrEqualThan(scope[j + offSet + 2 * n], s_i - scope[j + offSet]->minimum() + 1) == false)
                return CONFLICT;
            bool filtPrun2 = szBefore != scope[j + offSet + 2 * n]->size();

            if(filtPrun1 || filtPrun2) {
                if(!pruneList.contains(j)) {
                    pruneList.push(j);
                }
                hasFiltered = PROP;
            }
        }
    }
    return hasFiltered;
}

void NoOverlap2D::prop(int idx) {
    int v = idx % n;
    for(int i : overlappingBoxes.edges[v]) {
        if(mayOverlap(v, i) == false) {
            overlappingBoxes.removeEdge(v, i);
        }
    }
    if(boxesToCompute.contains(v) == false)
        boxesToCompute.push(v);
}


bool NoOverlap2D::energyCheck(int i) {
    int xm         = scope[i]->minimum();
    int xM         = scope[i]->maximum() + scope[i + 2 * n]->maximum();
    int ym         = scope[i + n]->minimum();
    int yM         = scope[i + n]->maximum() + scope[i + 3 * n]->maximum();
    int am         = scope[i + 2 * n]->minimum() * scope[i + 3 * n]->minimum();
    int xLengthMin = scope[i + 2 * n]->minimum();
    int yLengthMin = scope[i + 3 * n]->minimum();

    for(int j : overlappingBoxes.edges[i]) {
        xm = std::min(xm, scope[j]->minimum());
        xM = std::max(xM, scope[j]->maximum() + scope[j + 2 * n]->maximum());
        ym = std::min(ym, scope[j + n]->minimum());
        yM = std::max(yM, scope[j + n]->maximum() + scope[j + 3 * n]->maximum());
        am += scope[j + 2 * n]->minimum() * scope[j + 3 * n]->minimum();
        if(am > (xM - xm) * (yM - ym)) {
            return false;
        }
        xLengthMin = std::min(xLengthMin, scope[j + 2 * n]->minimum());
        yLengthMin = std::min(yLengthMin, scope[j + 3 * n]->minimum());
    }

    if(xLengthMin > 0 && yLengthMin > 0) {
        int maxNumberRectangles = ((xM - xm) / xLengthMin) * ((yM - ym) / yLengthMin);
        if(maxNumberRectangles < overlappingBoxes.edges[i].size() + 1) {
            return false;
        }
    }
    return true;
}
//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

NoOverlap2D::NoOverlap2D(Problem &p, std::string &_n, vec<Variable *> &_x, vec<Variable *> &_y, vec<Variable *> &_dx,
                         vec<Variable *> &_dy)
    : GlobalConstraint(p, _n, "NoOverlap 2D", 0), overlappingBoxes() {
    isPostponable = true;
    vec<Variable *> vars;

    _x.copyTo(vars);
    for(Variable *tmp : _y) vars.push(tmp);
    for(Variable *tmp : _dx) vars.push(tmp);
    for(Variable *tmp : _dy) vars.push(tmp);
    addToScope(vars);
    n = _x.size();
    boxesToCompute.growTo(n);
    pruneList.growTo(n);
    overlappingBoxes.setCapacity(n);

    for(int i = 0; i < n; i++) {
        for(int j = i + 1; j < n; j++) {
            if(mayOverlap(i, j))
                overlappingBoxes.addEdge(i, j);
        }
    }
}
