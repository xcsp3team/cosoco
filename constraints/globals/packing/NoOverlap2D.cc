#include "NoOverlap2D.h"

#include "solver/Solver.h"
using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool NoOverlap2D::isSatisfiedBy(vec<int> &t) {
    return true;
    for(int i = 0; i < n; i++)
        for(int j = i + 1; j < n; j++) {
            bool dx = t[i] + t[i + 2 * n] <= t[j] || t[j] + t[j + 2 * n] <= t[i];
            bool dy = t[i + n] + t[i + 3 * n] <= t[j + n] || t[j + n] + t[j + 3 * n] <= t[i + n];
            if(!dx && !dy)
                return false;
        }
    return true;
}

//----------------------------------------------
// Filtering
//----------------------------------------------
void NoOverlap2D::mark(int v) {
    if(v < 0 || v >= n)
        std::cerr << "NoOverlap2D::mark: v = " << v << std::endl;
    assert(v >= 0 && v < n);
    if(!inQueue[v]) {
        inQueue[v] = 1;
        toCompute.push_back(v);
    }
}

bool NoOverlap2D::fail() {
    for(int v : toCompute) inQueue[v] = 0;
    toCompute.clear();
    return false;
}

void NoOverlap2D::collectNeighbors(int i) {
    neighbors.clear();
    for(int j = 0; j < n; j++)
        if(j != i && mayOverlap(i, j))
            neighbors.push_back(j);
}

bool NoOverlap2D::filter(Variable *xx) {
    if(firstCall) {
        for(int i = 0; i < n; i++) mark(i);
        firstCall = false;
    } else
        for(int k = 0; k < 4 * n; k++)
            if(scope[k] == xx)
                mark(k % n);

    std::vector<int> cur;
    while(!toCompute.empty()) {
        cur.swap(toCompute);
        toCompute.clear();
        for(int i : cur) inQueue[i] = 0;
        for(int i : cur) {
            collectNeighbors(i);
            for(int j : neighbors)
                if(boxInstantiated(i) && boxInstantiated(j))
                    return fail();
            if(!energyCheck(i))
                return fail();
            if(prune(i) == CONFLICT)
                return fail();
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

    for(int i : neighbors) {
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
            if(solver->delValuesLE(scope[j + offSet], e_i - 1) == false)
                return CONFLICT;
            if(scope[j + offSet]->size() != szBefore) {
                mark(j);
                hasFiltered = PROP;
            }
            // and update the start of i to the end of j
            szBefore = scope[i + offSet]->size();
            if(solver->delValuesGE(scope[i + offSet], s_j - scope[i + 2 * n + offSet]->minimum() + 1) == false)
                return CONFLICT;
            bool filtPrun1 = scope[i + offSet]->size() != szBefore;

            szBefore = scope[i + offSet + 2 * n]->size();
            if(solver->delValuesGE(scope[i + offSet + 2 * n], s_j - scope[i + offSet]->minimum() + 1) == false)
                return CONFLICT;
            bool filtPrun2 = scope[i + offSet + 2 * n]->size() != szBefore;
            if(filtPrun1 || filtPrun2) {
                mark(i);
                hasFiltered = PROP;
            }
        }
        if(s_j < e_i) {   // if j starts before i ends
            // then update the start of i to the end of j
            int szBefore = scope[i + offSet]->size();
            if(solver->delValuesLE(scope[i + offSet], e_j - 1) == false)
                return CONFLICT;
            if(szBefore != scope[i + offSet]->size()) {
                mark(i);
                hasFiltered = PROP;
            }

            // and update the start of j to the end of i
            szBefore = scope[j + offSet]->size();
            if(solver->delValuesGE(scope[j + offSet], s_i - scope[j + 2 * n + offSet]->minimum() + 1) == false)
                return CONFLICT;
            bool filtPrun1 = szBefore != scope[j + offSet]->size();
            // and update the size of j to the space between the earliest start of i and the earliest start of j
            szBefore = scope[j + offSet + 2 * n]->size();
            if(solver->delValuesGE(scope[j + offSet + 2 * n], s_i - scope[j + offSet]->minimum() + 1) == false)
                return CONFLICT;
            bool filtPrun2 = szBefore != scope[j + offSet + 2 * n]->size();

            if(filtPrun1 || filtPrun2) {
                mark(j);
                hasFiltered = PROP;
            }
        }
    }
    return hasFiltered;
}


bool NoOverlap2D::energyCheck(int i) {
    int64_t xm         = scope[i]->minimum();
    int64_t xM         = scope[i]->maximum() + scope[i + 2 * n]->maximum();
    int64_t ym         = scope[i + n]->minimum();
    int64_t yM         = scope[i + n]->maximum() + scope[i + 3 * n]->maximum();
    int64_t am         = scope[i + 2 * n]->minimum() * scope[i + 3 * n]->minimum();
    int64_t xLengthMin = scope[i + 2 * n]->minimum();
    int64_t yLengthMin = scope[i + 3 * n]->minimum();

    for(int j : neighbors) {
        xm = std::min(xm, static_cast<int64_t>(scope[j]->minimum()));
        xM = std::max(xM, static_cast<int64_t>(scope[j]->maximum()) + scope[j + 2 * n]->maximum());
        ym = std::min(ym, static_cast<int64_t>(scope[j + n]->minimum()));
        yM = std::max(yM, static_cast<int64_t>(scope[j + n]->maximum()) + scope[j + 3 * n]->maximum());
        am += scope[j + 2 * n]->minimum() * scope[j + 3 * n]->minimum();
        if(am > (xM - xm) * (yM - ym)) {
            return false;
        }
        xLengthMin = std::min(xLengthMin, static_cast<int64_t>(scope[j + 2 * n]->minimum()));
        yLengthMin = std::min(yLengthMin, static_cast<int64_t>(scope[j + 3 * n]->minimum()));
    }

    if(xLengthMin > 0 && yLengthMin > 0) {
        uint64_t maxNumberRectangles = ((xM - xm) / xLengthMin) * ((yM - ym) / yLengthMin);
        if(maxNumberRectangles < neighbors.size() + 1) {
            return false;
        }
    }
    return true;
}
//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

NoOverlap2D::NoOverlap2D(Problem &p, vec<Variable *> &_x, vec<Variable *> &_y, vec<Variable *> &_dx, vec<Variable *> &_dy)
    : GlobalConstraint(p, "NoOverlap 2D", 0) {
    isPostponable = true;
    vec<Variable *> vars;

    _x.copyTo(vars);
    for(Variable *tmp : _y) vars.push(tmp);
    for(Variable *tmp : _dx) vars.push(tmp);
    for(Variable *tmp : _dy) vars.push(tmp);
    addToScope(vars);
    n = _x.size();
    inQueue.assign(n, 0);
}
