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


bool AllDifferent::filter(Variable *dummy) {
    //    VC  filtering
    if(variableConsistency)
        return basicFilter(dummy);

    // Bound consistency
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


bool AllDifferent::basicFilter(Variable *x) {
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


//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------

AllDifferent::AllDifferent(Problem &p, std::string nn, vec<Variable *> &vars)
    : GlobalConstraint(p, nn, "All Different", vars), variableConsistency(false) {
    int sz = scope.size();
    t.growTo(2 * sz + 2, 0);
    d.growTo(2 * sz + 2, 0);
    h.growTo(2 * sz + 2, 0);
    bounds.growTo(2 * sz + 2, 0);

    // Initialisation
    for(int i = 0; i < sz; i++) {
        Interval *it = new Interval(scope[i]);
        interval.push(it);
        minsorted.push(it);
        maxsorted.push(it);
    }
}




//----------------------------------------------------------
// Filtering
//----------------------------------------------------------


// ---- Filtering using IJCAI'03 paper
//      A Fast and Simple Algorithm for Bound Consistency Of the AllDifferent Constraint.


void AllDifferent::sortIt() {
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


void AllDifferent::pathset(vec<int> &tab, int start, int end, int to) {
    int next = start;
    int prev = next;
    while(prev != end) {
        next      = tab[prev];
        tab[prev] = to;
        prev      = next;
    }
}


int AllDifferent::pathmin(vec<int> &tab, int i) {
    while(tab[i] < i) i = tab[i];
    return i;
}


int AllDifferent::pathmax(vec<int> &tab, int i) {
    while(tab[i] > i) i = tab[i];
    return i;
}


bool AllDifferent::filterLower(bool &again) {
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


bool AllDifferent::filterUpper(bool &again) {
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