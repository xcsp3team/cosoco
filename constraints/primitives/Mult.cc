//
// Created by audemard on 30/01/2026.
//

#include "Mult.h"

#include "Solver.h"
using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool Mult3EQ::isSatisfiedBy(vec<int> &tuple) { return tuple[0] * tuple[1] == tuple[2]; }


//----------------------------------------------
// Filtering
//----------------------------------------------

static int smallestIntegerGE(int c, int k) {
    if(c == 0)
        return k <= 0 ? INT8_MIN : INT_MAX;
    int    limit = k / c;
    double ll    = k / static_cast<double>(c);
    if(k > 0 && k % c != 0)
        limit += (ll < 0 ? -1 : 1);
    return limit;
}

static int greatestIntegerLE(int c, int k) {   // c*x <= k
    if(c == 0)
        return k >= 0 ? INT_MIN : INT_MAX;
    int    limit = k / c;
    double ll    = k / static_cast<double>(c);
    if(k < 0 && k % c != 0)
        limit += (ll < 0 ? -1 : 1);
    return limit;
}


bool Mult3EQ::reviseGE(Variable *x1, Variable *x2, int k) {
    if(x2->containsValue(0)) {
        if(0 >= k)
            return true;
        if(solver->delVal(x2, 0) == false)
            return false;
    }
    assert(x2->containsValue(0) == false);

    if(x2->minimum() > 0) {
        // all values in d2 are positive
        int b = k < 0 ? smallestIntegerGE(x2->minimum(), k) : smallestIntegerGE(x2->maximum(), k);
        return solver->delValuesLowerOrEqualThan(x1, b - 1);
    }

    if(x2->maximum() < 0) {
        // all values in d2 are negative
        int b = k > 0 ? greatestIntegerLE(-x2->minimum(), -k) : greatestIntegerLE(-x2->maximum(), -k);
        return solver->delValuesGreaterOrEqualThan(x1, b + 1);
    }

    return solver->delValuesInRange(x1, greatestIntegerLE(-x2->minimum(), -k) + 1, smallestIntegerGE(x2->maximum(), k));
}

bool Mult3EQ::reviseLE(Variable *x1, Variable *x2, int k) {
    if(x2->containsValue(0)) {
        if(0 <= k)
            return true;
        if(solver->delVal(x2, 0) == false)
            return false;
    }
    assert(x2->containsValue(0) == false);

    if(x2->minimum() > 0) {
        // all values in d2 are positive
        int b = k > 0 ? greatestIntegerLE(x2->minimum(), k) : greatestIntegerLE(x2->maximum(), k);
        return solver->delValuesGreaterOrEqualThan(x1, b + 1);
    }

    if(x2->maximum() < 0) {
        // all values in d2 are negative
        int b = k < 0 ? smallestIntegerGE(-x2->minimum(), -k) : smallestIntegerGE(-x2->maximum(), -k);
        return solver->delValuesLowerOrEqualThan(x1, b - 1);
    }

    return solver->delValuesInRange(x1, greatestIntegerLE(x2->maximum(), k) + 1, smallestIntegerGE(-x2->minimum(), -k));
}


bool Mult3EQ::enforceMulGE(Variable *x1, Variable *y1, int k) { return reviseGE(x1, y1, k) && reviseGE(y1, x1, k); }
bool Mult3EQ::enforceMulLE(Variable *x1, Variable *y1, int k) { return reviseLE(x1, y1, k) && reviseLE(y1, x1, k); }


bool Mult3EQ::filter(Variable *dummy) {
    if(tooLarge(x->size(), y->size())) {   // hard coding // TODO what about AC Guaranteed?
        int v1 = x->minimum() * y->minimum(), v2 = x->minimum() * y->maximum();
        int v3 = x->maximum() * y->minimum(), v4 = x->maximum() * y->maximum();
        int min1 = std::min(v1, v2), max1 = std::max(v1, v2);
        int min2 = std::min(v3, v4), max2 = std::max(v3, v4);
        if(solver->delValuesLowerOrEqualThan(z, std::min(min1, min2) - 1) == false ||
           solver->delValuesGreaterOrEqualThan(z, std::max(max1, max2) + 1) == false)
            return false;

        if(enforceMulGE(x, y, z->minimum()) && enforceMulLE(x, y, z->maximum()) == false)
            return false;
        if(tooLarge(x->size(), y->size()))   // otherwise we keep filtering below
            return true;
    }
    bool found = false;
    if(z->containsValue(0) == false || z->containsValue(0) == false) {
        // if 0 is present in dy and dz, all values of x are supported
        for(int a : x->domain) {
            found  = false;
            int va = x->domain.toVal(a);
            if(va == 0) {
                if(z->containsValue(0) == false && solver->delIdv(x, a) == false)
                    return false;
                continue;
            }
            if(y->containsIdv(rx[a]) && z->containsValue(va * y->domain.toVal(rx[a])))
                continue;
            for(int b : y->domain) {
                int vc = va * y->domain.toVal(b);
                if((va > 0 && vc > z->maximum()) || (va < 0 && vc < z->minimum()))
                    break;
                if(z->containsValue(vc)) {
                    rx[a] = b;
                    found = true;
                    break;
                }
            }
            if(found == false && solver->delIdv(x, a) == false) {
                return false;
            }
        }
    }


    if(x->containsValue(0) == false || z->containsValue(0) == false)
        // if 0 is present in dx and dz, all values of y are supported
        for(int b : y->domain) {
            found  = false;
            int vb = y->domain.toVal(b);
            if(vb == 0) {
                if(z->containsValue(0) == false && solver->delIdv(y, b) == false)
                    return false;
                continue;
            }
            if(x->containsIdv(ry[b]) && z->containsValue(vb * x->domain.toVal(ry[b])))
                continue;
            for(int a : x->domain) {
                int vc = vb * x->domain.toVal(a);
                if((vb > 0 && vc > z->maximum()) || (vb < 0 && vc < z->minimum()))
                    break;
                if(z->containsValue(vc)) {
                    ry[b] = a;
                    found = true;
                    break;
                }
            }
            if(found == false && solver->delIdv(y, b) == false) {
                return false;
            }
        }


    for(int c : z->domain) {
        found  = false;
        int vc = z->domain.toVal(c);
        if(vc == 0) {
            if(x->containsValue(0) == false && y->containsValue(0) == false && solver->delIdv(z, c) == false)
                return false;
            continue;
        }
        if(rzx[c] != -1 && x->containsIdv(rzx[c]) && y->containsIdv(rzy[c]))
            continue;
        for(int a : x->domain) {
            int va = x->domain.toVal(a);
            if(va == 0)   // because it involves vc=0, and vc = 0 already handled
                continue;
            int vb = vc / va;
            if(va > 0 && vc > 0 && va * y->minimum() > vc)   // TODO other ways of breaking?
                break;
            if(vc % va == 0 && y->containsValue(vb)) {
                rzx[c] = a;
                rzy[c] = y->domain.toIdv(vb);
                found  = true;
                break;
            }
        }
        if(found == false && solver->delIdv(z, c) == false) {
            return false;
        }
    }
    return true;
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

Mult3EQ::Mult3EQ(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz) : Ternary(p, n, xx, yy, zz) {
    type = "X * Y = Z";
    std::cout << xx->_name << " " << yy->_name << " " << zz->_name << "\n";
    rx.growTo(x->domain.maxSize(), 0);
    ry.growTo(y->domain.maxSize(), 0);
    rzx.growTo(z->domain.maxSize(), -1);
    rzy.growTo(z->domain.maxSize(), -1);
}
