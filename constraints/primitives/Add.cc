#include "Add.h"

#include "solver/Solver.h"

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool Lt::isSatisfiedBy(vec<int> &tuple) { return tuple[0] + k < tuple[1]; }

bool Add3::isSatisfiedBy(vec<int> &tuple) { return tuple[0] + tuple[1] == tuple[2]; }

//----------------------------------------------
// Filtering
//----------------------------------------------

bool Lt::filter(Variable *dummy) {
    if(solver->isAssigned(x) == false)
        if(solver->delValuesGreaterOrEqualThan(x, y->maximum() - k) == false)
            return false;

    if(solver->isAssigned(y) == false)
        if(solver->delValuesLowerOrEqualThan(y, x->minimum() + k) == false)
            return false;
    return true;
}

bool Add3::filter(Variable *dummy) {
    if(x->size() == 1)
        return solver->enforceEQ(z, y, x->value());
    if(y->size() == 1)
        return solver->enforceEQ(z, x, y->value());
    if(z->size() == 1)
        return solver->enforceAddEQ(x, y, z->value());

    if(tooLarge(x->size(), y->size())) {
        if(solver->delValuesLowerOrEqualThan(z, x->minimum() + y->minimum() - 1) == false)
            return false;
        if(solver->delValuesGreaterOrEqualThan(z, x->maximum() + y->maximum() + 1) == false)
            return false;
        if(solver->enforceAddGE(x, y, z->minimum()) == false)
            return false;
        if(solver->enforceAddLE(x, y, z->maximum()) == false)
            return false;
        if(tooLarge(x->size(), y->size()))   // otherwise we keep filtering below
            return true;
    }

#define isConnex(w) ((w)->maximum() - (w)->minimum() + 1 == (w)->size())

#define enclose(w, minValueIncluded, maxValueIncluded) \
    ((w)->minimum() <= (minValueIncluded) && (maxValueIncluded) <= (w)->maximum())

    bool avoidx = false, avoidy = false;
    if(isConnex(z)) {
        avoidx = enclose(z, x->minimum() + y->minimum(), x->maximum() + y->minimum()) ||
                 enclose(z, x->minimum() + y->maximum(), x->maximum() + y->maximum());
        avoidy = enclose(z, y->minimum() + x->minimum(), y->maximum() + x->minimum()) ||
                 enclose(z, y->minimum() + x->maximum(), y->maximum() + x->maximum());
    }

    bool found = false;
    if(avoidx == false)
        for(int idvx : x->domain) {   // Del value in
            found  = false;
            int vx = x->domain.toVal(idvx);
            if(y->containsIdv(rx[idvx]) && z->containsValue(vx + y->domain.toVal(rx[idvx])))
                continue;
            if(y->size() <= z->size())
                for(int idvy : y->domain) {
                    int vz = vx + y->domain.toVal(idvy);
                    if(vz > z->maximum())
                        break;
                    if(z->containsValue(vz)) {
                        rx[idvx] = idvy;
                        found    = true;
                        break;
                    }
                }
            else
                for(int idvz : z->domain) {
                    int vy = z->domain.toVal(idvz) - vx;
                    if(vy > y->maximum())
                        break;
                    if(y->containsValue(vy)) {
                        rx[idvx] = y->domain.toIdv(vy);
                        found    = true;
                        break;
                    }
                }
            if(found == false && solver->delIdv(x, idvx) == false)
                return false;
        }

    if(avoidy == false)
        for(int idvy : y->domain) {
            found  = false;
            int vy = y->domain.toVal(idvy);
            if(x->containsIdv(ry[idvy]) && z->containsValue(vy + x->domain.toVal(ry[idvy])))
                continue;
            if(x->size() <= z->size())
                for(int idvx : x->domain) {
                    int vz = vy + x->domain.toVal(idvx);
                    if(vz > z->maximum())
                        break;
                    if(z->containsValue(vz)) {
                        ry[idvy] = idvx;
                        found    = true;
                    }
                }
            else
                for(int idvz : z->domain) {
                    int vx = z->domain.toVal(idvz) - vy;
                    if(vx > x->maximum())
                        break;
                    if(x->containsValue(vx)) {
                        ry[idvy] = x->domain.toIdv(vx);
                        found    = true;
                        break;
                    }
                }
            if(found == false && solver->delIdv(y, idvy) == false)
                return false;
        }


    for(int idvz : z->domain) {
        found  = false;
        int vz = z->domain.toVal(idvz);
        if(x->containsIdv(rzx[idvz]) && y->containsValue(vz - x->domain.toVal(rzx[idvz])))
            continue;
        if(x->size() <= y->size())
            for(int idvx : reverse(x->domain)) {   // Reverse to stop iteration with condition vy > y->maximum()
                int vy = vz - x->domain.toVal(idvx);
                if(vy > y->maximum())
                    break;
                if(y->containsValue(vy)) {
                    rzx[idvz] = idvx;
                    found     = true;
                    break;
                }
            }
        else
            for(int idvy : reverse(y->domain)) {   // Reverse to stop iteration with condition vx > x->maximum()
                int vx = vz - y->domain.toVal(idvy);
                if(vx > x->maximum())
                    break;
                if(x->domain.containsValue(vx)) {
                    rzx[idvz] = x->domain.toIdv(vx);
                    found     = true;
                    break;
                }
            }

        if(found == false && solver->delIdv(z, idvz) == false)
            return false;
    }

    return true;
}

//----------------------------------------------
// Construction and initalisation
//----------------------------------------------

Lt::Lt(Problem &p, std::string n, Variable *xx, Variable *yy, int kk) : Binary(p, n, xx, yy), k(kk) { type = "(X - Y < k)"; }

Add3::Add3(Problem &p, std::string n, Variable *x, Variable *y, Variable *z) : Ternary(p, n, x, y, z) {
    type = "X + Y = Z";
    rx.growTo(x->domain.maxSize(), 0);
    ry.growTo(y->domain.maxSize(), 0);
    rzx.growTo(z->domain.maxSize(), 0);
}