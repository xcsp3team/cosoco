#include "AllDifferent.h"

#include "mtl/Sort.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------------------
// check validity
//----------------------------------------------------------


bool AllDifferent::isSatisfiedBy(vec<int> &tuple) {
    for(int posx = 0; posx < tuple.size(); posx++) {
        int v = tuple[posx];
        for(int posy = posx + 1; posy < tuple.size(); posy++)
            if(v == tuple[posy])
                return false;
    }
    return true;
}


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------
bool AllDifferentBasic::filter(Variable *x) {
    if(x->size() > 1)
        return true;
    int v = x->value();
    for(Variable *y : scope) {
        if(y == x)
            continue;
        if(solver->delVal(y, v) == false)
            return false;
    }
    return true;
}


bool AllDifferentInterval::filter(Variable *dummy) {
    bool again;
    do {
        again = false;   // Reach fi point?
        sortIt();
        if(filterLower(again) == false)
            return false;   // Contradiction
        if(filterUpper(again) == false)
            return false;   // Contradiction
    } while(again);
    return true;
}


bool AllDifferentPermutation::filter(Variable *dummy) {
    int level = solver->decisionLevel();
    for(int i = unfixedVars.size() - 1; i >= 0; i--) {
        Variable *x = scope[unfixedVars[i]];
        if(x->size() == 1) {
            int idv = x->domain[0];
            unfixedVars.del(scope.firstOccurrenceOf(x), level);
            unfixedIdxs.del(idv, level);
            for(int j = unfixedVars.size() - 1; j >= 0; j--) {
                Variable *y = scope[unfixedVars[j]];
                if(solver->delIdv(y, idv) == false) {
                    return false;
                }
                if(y->size() == 1)
                    i = std::max(i, j + 1);   // +1 because i-- before a new iteration
            }
        }
    }

    for(int idv : reverse(unfixedIdxs)) {
        if(sentinels1[idv]->containsIdv(idv) == false) {
            Variable *x = findSentinel(idv, sentinels2[idv]);
            if(x != nullptr)
                sentinels1[idv] = x;
            else {
                x = sentinels2[idv];
                if(solver->assignToIdv(x, idv) == false)
                    return false;
                unfixedVars.del(scope.firstOccurrenceOf(x), level);
                unfixedIdxs.del(idv, level);
            }
        }
        assert(sentinels1[idv]->size() > 1);

        if(sentinels2[idv]->containsIdv(idv)) {
            Variable *x = findSentinel(idv, sentinels1[idv]);
            if(x != nullptr)
                sentinels2[idv] = x;
            else {
                x = sentinels1[idv];
                solver->assignToIdv(x, idv);
                unfixedVars.del(scope.firstOccurrenceOf(x), level);
                unfixedIdxs.del(idv, level);
            }
        }
    }
    return true;
}


//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------

AllDifferentInterval::AllDifferentInterval(Problem &p, std::string nn, vec<Variable *> &vars) : AllDifferent(p, nn, vars) {
    int sz = scope.size();
    t.growTo(2 * sz + 2, 0);
    d.growTo(2 * sz + 2, 0);
    h.growTo(2 * sz + 2, 0);
    bounds.growTo(2 * sz + 2, 0);

    // Initialisation
    for(int i = 0; i < sz; i++) {
        auto *it = new Interval(scope[i]);
        interval.push(it);
        minsorted.push(it);
        maxsorted.push(it);
    }
}

AllDifferentPermutation::AllDifferentPermutation(Problem &p, std::string n, vec<Variable *> &vars) : AllDifferent(p, n, vars) {
    unfixedVars.setCapacity(scope.size(), true);
    unfixedIdxs.setCapacity(scope[0]->domain.maxSize(), true);
    sentinels1.growTo(scope[0]->domain.maxSize(), scope[0]);
    sentinels2.growTo(scope[0]->domain.maxSize(), scope.last());
}

//----------------------------------------------------------
// Internal functions
//----------------------------------------------------------

void AllDifferentPermutation::notifyDeleteDecision(Variable *x, int v, Solver &s) {
    unfixedIdxs.restoreLimit(s.decisionLevel() + 1);
    unfixedVars.restoreLimit(s.decisionLevel() + 1);
}

Variable *AllDifferentPermutation::findSentinel(int idv, Variable *otherSentinel) {
    for(int posx : unfixedVars) {
        Variable *x = scope[posx];
        if(x != otherSentinel && x->containsIdv(idv))
            return x;
    }
    return nullptr;
}


void AllDifferentInterval::sortIt() {
    int sz = scope.size();
    for(int i = 0; i < sz; i++) {
        Variable *x     = interval[i]->x;
        interval[i]->lb = x->minimum();
        interval[i]->ub = x->maximum();
    }

    sort(minsorted, MinOrder());
    sort(maxsorted, MaxOrder());

    int min = minsorted[0]->lb;
    int max = maxsorted[0]->ub + 1;

    int last  = min - 2;
    int nb    = 0;
    bounds[0] = last;
    int i = 0, j = 0;
    while(true) {
        if(i < sz && min <= max) {
            if(min != last)
                bounds[++nb] = last = min;
            minsorted[i]->minrank = nb;
            if(++i < sz)
                min = minsorted[i]->lb;
        } else {
            if(max != last)
                bounds[++nb] = last = max;
            maxsorted[j]->maxrank = nb;
            if(++j == sz)
                break;
            max = maxsorted[j]->ub + 1;
        }
    }
    nbBounds       = nb;
    bounds[nb + 1] = bounds[nb] + 2;
}


void AllDifferentInterval::pathset(vec<int> &tab, int start, int end, int to) {
    int next = start;
    int prev = next;
    while(prev != end) {
        next      = tab[prev];
        tab[prev] = to;
        prev      = next;
    }
}


int AllDifferentInterval::pathmin(vec<int> &tab, int i) {
    while(tab[i] < i) i = tab[i];
    return i;
}


int AllDifferentInterval::pathmax(vec<int> &tab, int i) {
    while(tab[i] > i) i = tab[i];
    return i;
}


bool AllDifferentInterval::filterLower(bool &again) {
    int sz = scope.size();
    for(int i = 1; i <= nbBounds + 1; i++) {
        t[i] = h[i] = i - 1;
        d[i]        = bounds[i] - bounds[i - 1];
    }


    for(int i = 0; i < sz; i++) {
        int x = maxsorted[i]->minrank;
        int y = maxsorted[i]->maxrank;
        int z = pathmax(t, x + 1);
        int j = t[z];

        if(--d[z] == 0) {
            t[z] = z + 1;
            z    = pathmax(t, t[z]);
            t[z] = j;
        }

        pathset(t, x + 1, z, z);
        if(d[z] < bounds[z] - bounds[y])
            return false;

        if(h[x] > x) {
            int       w      = pathmax(h, h[x]);
            Variable *cur    = maxsorted[i]->x;
            int       before = cur->size();
            if(solver->delValuesLowerOrEqualThan(cur, bounds[w] - 1) == false)
                return false;
            if(before > cur->size()) {
                again            = true;
                maxsorted[i]->lb = cur->domain.minimum();
            }
            pathset(h, x, w, w);
        }
        if(d[z] == bounds[z] - bounds[y]) {
            pathset(h, h[y], j - 1, y);
            h[y] = j - 1;
        }
    }
    return true;
}


bool AllDifferentInterval::filterUpper(bool &again) {
    int sz = scope.size();
    for(int i = 0; i <= nbBounds; i++) {
        t[i] = h[i] = i + 1;
        d[i]        = bounds[i + 1] - bounds[i];
    }

    for(int i = sz - 1; i >= 0; i--) {
        int x = minsorted[i]->maxrank;
        int y = minsorted[i]->minrank;
        int z = pathmin(t, x - 1);
        int j = t[z];
        if(--d[z] == 0) {
            t[z] = z - 1;
            z    = pathmin(t, t[z]);
            t[z] = j;
        }
        pathset(t, x - 1, z, z);

        if(d[z] < bounds[y] - bounds[z]) {
            return false;
        }
        if(h[x] < x) {
            int       w      = pathmin(h, h[x]);
            Variable *cur    = minsorted[i]->x;
            int       before = cur->size();
            if(solver->delValuesGreaterOrEqualThan(cur, bounds[w]) == false)
                return false;
            if(before > cur->size()) {
                minsorted[i]->ub = cur->domain.maximum();
                again            = true;
            }
            pathset(h, x, w, w);
        }
        if(d[z] == bounds[y] - bounds[z]) {
            pathset(h, h[y], j + 1, y);
            h[y] = j + 1;
        }
    }


    return true;
}
