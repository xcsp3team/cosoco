#include "constraints/primitives/xTimesyEQz.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool xTimesyEQz::isSatisfiedBy(vec<int> &tuple) { return tuple[0] * tuple[1] == tuple[2]; }


//----------------------------------------------
// Filtering
//----------------------------------------------



bool xTimesyEQz::filter(Variable *dummy) {
/*    if (x->size() * y->size() > 200) { // hard coding
        int v1 = x->minimum() * y->minimum(), v2 = x->minimum() * y->maximum();
        int v3 = x->maximum() * y->minimum(), v4 = x->maximum() * y->maximum();
        int min1 = v1 < v2 ? v1 : v2, max1 = v1 > v2 ? v1 : v2;
        int min2 = v3 < v4 ? v3 : v4, max2 = v3 > v4 ? v3 : v4;
        if (dz.removeValuesLT(Math.min(min1, min2)) == false || dz.removeValuesGT(Math.max(max1, max2)) == false)
            return false;
        return PrimitiveBinary.enforceMulGE(dx, dy, dz.firstValue()) && PrimitiveBinary.enforceMulLE(dx, dy, dz.lastValue());
    }
    if (!dy.isPresentValue(0) || !dz.isPresentValue(0)) // if 0 is present in dy and dz, all values of x are supported
    extern: for (int a = dx.first(); a != -1; a = dx.next(a)) {
        int va = dx.toVal(a);
        if (va == 0) {
            if (!dz.isPresentValue(0) && dx.remove(a) == false)
                return false;
            continue;
        }
        if (dy.present(rx[a]) && dz.isPresentValue(va * dy.toVal(rx[a])))
            continue;
        for (int b = dy.first(); b != -1; b = dy.next(b)) {
            int vc = va * dy.toVal(b);
            if ((va > 0 && vc > dz.lastValue()) || (va < 0 && vc < dz.firstValue()))
                break;
            if (dz.isPresentValue(vc)) {
                rx[a] = b;
                continue extern;
            }
        }
        if (dx.remove(a) == false)
            return false;
    }
    if (!dx.isPresentValue(0) || !dz.isPresentValue(0)) // if 0 is present in dx and dz, all values of y are supported
    extern: for (int b = dy.first(); b != -1; b = dy.next(b)) {
        int vb = dy.toVal(b);
        if (vb == 0) {
            if (!dz.isPresentValue(0) && dy.remove(b) == false)
                return false;
            continue;
        }
        if (dx.present(ry[b]) && dz.isPresentValue(vb * dx.toVal(ry[b])))
            continue;
        for (int a = dx.first(); a != -1; a = dx.next(a)) {
            int vc = vb * dx.toVal(a);
            if ((vb > 0 && vc > dz.lastValue()) || (vb < 0 && vc < dz.firstValue()))
                break;
            if (dz.isPresentValue(vc)) {
                ry[b] = a;
                continue extern;
            }
        }
        if (dy.remove(b) == false)
            return false;
    }
    extern: for (int c = dz.first(); c != -1; c = dz.next(c)) {
        int vc = dz.toVal(c);
        if (vc == 0) {
            if (!dx.isPresentValue(0) && !dy.isPresentValue(0) && dz.remove(c) == false)
                return false;
            continue;
        }
        if (rzx[c] != -1 && dx.present(rzx[c]) && dy.present(rzy[c]))
            continue;
        for (int a = dx.first(); a != -1; a = dx.next(a)) {
            int va = dx.toVal(a);
            if (va == 0) // because vc = 0 already handled (and we need to be careful about division by zero
                continue;
            int vb = vc / va;
            if (va > 0 && vc > 0 && va * dy.firstValue() > vc) // TODO other ways of breaking?
                break;
            if (vc % va == 0 && dy.isPresentValue(vb)) {
                rzx[c] = a;
                rzy[c] = dy.toIdx(vb);
                continue extern;
            }
        }
        if (dz.remove(c) == false)
            return false;
    }
    return true;
}



*/

    // Filtering already done in special cases where x=0 or y=0
    if(solver->isAssigned(x) && x->value() == 0)
        return solver->assignToVal(z, 0);
    if(solver->isAssigned(y) && y->value() == 0)
        return solver->assignToVal(z, 0);

    int xmin = x->minimum(), xmax = x->maximum();
    int ymin = y->minimum(), ymax = y->maximum();

    // Update z bounds
    if(xmin >= 0 && ymin >= 0) {   // z>=0
        solver->delValuesLowerOrEqualThan(z, xmin * ymin - 1);
        solver->delValuesGreaterOrEqualThan(z, xmax * ymax + 1);
    } else if(xmax < 0 && ymax < 0) {   // Z>=0
        solver->delValuesLowerOrEqualThan(z, xmax * ymax - 1);
        solver->delValuesGreaterOrEqualThan(z, xmin * ymin + 1);
    } else if((xmin > 0 && ymax < 0) || (xmax < 0 && ymin > 0)) {   // Z<0
        int a = xmin * ymax;
        int b = xmax * ymin;
        solver->delValuesLowerOrEqualThan(z, std::min(a, b) - 1);
        solver->delValuesGreaterOrEqualThan(z, std::max(a, b) + 1);
    }
    if(z->size() == 0)   // Here to avoid to if()....)
        return false;
    if(x->size() == 0) return false;
    if(y->size() == 0) return false;

    if(solver->isAssigned(x) && instantiated(x, y) == false)
        return false;
    if(solver->isAssigned(y) && instantiated(y, x) == false)
        return false;
    if(z->size() == 0) return false;
    return true;
}


bool xTimesyEQz::instantiated(Variable *w, Variable *t) {
    int vw = w->value();
    if(solver->isAssigned(t))
        return solver->assignToVal(z, vw * t->value());
    if(solver->isAssigned(z)) {
        int approx = (z->value() / vw);
        if(approx * vw != z->value())
            return false;
        return solver->assignToVal(y, approx);
    }
    int v1 = vw * t->minimum();
    int v2 = vw * t->maximum();
    if(solver->delValuesLowerOrEqualThan(z, std::min(v1, v2) - 1) == false)
        return false;
    return solver->delValuesGreaterOrEqualThan(z, std::max(v1, v2) + 1);
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

xTimesyEQz::xTimesyEQz(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz) : Ternary(p, n, xx, yy, zz) {
    type = "(X * Y = Z)";
}
