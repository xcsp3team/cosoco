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
    prop(varIdx);

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
        for(int k = 0; k < boxesToCompute.size(); k++) {
            int i = boxesToCompute.getQuick(k);
            if(!lcg())
                energyCheck(i);
            hasFiltered |= prune(i);
        }
        boxesToCompute.resetQuick();
        for(int k : pruneList) {
            prop(k);
        }
    }
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

bool NoOverlap2D::prune(int j) {
    bool hasFiltered = false;

    for(int i : neighbours[j]) {
        if(doOverlap(i, j, true))
            hasFiltered |= doFiltering(i, j, false);
        if(doOverlap(i, j, false))
            hasFiltered |= doFiltering(i, j, true);
    }
    return hasFiltered;
}


bool NoOverlap2D::doFiltering(int i, int j, bool hori) {
    bool hasFiltered = false;
    int  offSet      = hori ? 0 : n;
    int  s_i         = scope[i + offSet]->maximum();                                          // latest start of i
    int  e_i         = scope[i + offSet]->minimum() + scope[i + 2 * n + offSet]->minimum();   // earliest end of i
    int  s_j         = scope[j + offSet]->maximum();                                          // latest start of j
    int  e_j         = scope[j + offSet]->minimum() + scope[j + 2 * n + offSet]->minimum();   // earliest end of j
    if(s_i < e_i || s_j < e_j) {   // if at least one mandatory part is not empty
        if(e_j > s_i) {            // if j ends after i starts
            // then update the start of j to the end of i
            if(scope[j + offSet].updateLowerBound(e_i, this, explainFilter(j, i))) {
                if(!pruneList.contains(j)) {
                    pruneList.add(j);
                }
                hasFiltered = true;
            }
            // and update the start of i to the end of j
            bool filtPrun1 =
                scope[i + offSet].updateUpperBound(s_j - scope[i + 2 * n + offSet]->minimum(), this, explainFilter(i, j));
            // and update the size of i to the space between the earliest start of j and the earliest start of i
            bool filtPrun2 =
                scope[i + offSet + 2 * n].updateUpperBound(s_j - scope[i + offSet]->minimum(), this, explainFilter(i, j));
            if(filtPrun1 || filtPrun2) {
                if(!pruneList.contains(i)) {
                    pruneList.add(i);
                }
                hasFiltered = true;
            }
        }
        if(s_j < e_i) {   // if j starts before i ends
            // then update the start of i to the end of j
            if(scope[i + offSet].updateLowerBound(e_j, this, explainFilter(i, j))) {
                if(!pruneList.contains(i)) {
                    pruneList.add(i);
                }
                hasFiltered = true;
            }
            // and update the start of j to the end of i
            bool filtPrun1 =
                scope[j + offSet].updateUpperBound(s_i - scope[j + 2 * n + offSet]->minimum(), this, explainFilter(j, i));
            // and update the size of j to the space between the earliest start of i and the earliest start of j
            bool filtPrun2 =
                scope[j + offSet + 2 * n].updateUpperBound(s_i - scope[j + offSet]->minimum(), this, explainFilter(j, i));
            if(filtPrun1 || filtPrun2) {
                if(!pruneList.contains(j)) {
                    pruneList.add(j);
                }
                hasFiltered = true;
            }
        }
    }
    return hasFiltered;
}

void NoOverlap2D::prop(int idx) {
    int          v    = idx % n;
    ISetIterator iter = overlappingBoxes.getNeighborsOf(v).iterator();
    while(iter.hasNext()) {
        int i = iter.nextInt();
        if(!mayOverlap(v, i)) {
            overlappingBoxes.removeEdge(v, i);
        }
    }
    if(!boxesToCompute.contains(v)) {
        boxesToCompute.add(v);
    }
}
//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

NoOverlap2D::NoOverlap2D(Problem &p, std::string _n, vec<Variable *> &_x, vec<Variable *> &_y, vec<Variable *> &_dx,
                         vec<Variable *> &_dy)
    : GlobalConstraint(p, _n, "Disjunctive2DVar", 0) {
    vec<Variable *> vars;

    _x.copyTo(vars);
    for(Variable *tmp : _y) vars.push(tmp);
    for(Variable *tmp : _dx) vars.push(tmp);
    for(Variable *tmp : _dy) vars.push(tmp);
    addToScope(vars);
    n = _x.size();
    boxesToCompute.growTo(n);
    pruneList.growTo(n);
    neighbours.growTo(n);
    for(int i = 0; i < n; i++) {
        for(int j = i + 1; j < n; j++) {
            if(mayOverlap(i, j)) {
                neighbours[i].push(j);
                neighbours[j].push(i);
            }
        }
    }
}
