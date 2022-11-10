#include "constraints/primitives/DistXYeqZ.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool DistXYeqZ::isSatisfiedBy(vec<int> &tuple) { return abs(tuple[0] - tuple[1]) == tuple[2]; }


//----------------------------------------------
// Filtering
//----------------------------------------------

bool DistXYeqZ::supportX(Variable *t, int v, int a, int b, int c) {
    if(t->containsValue(v)) {
        rx[a]  = b;
        ry[b]  = a;
        rzx[c] = a;
        rzy[c] = b;
        return true;
    }
    return false;
}

bool DistXYeqZ::supportY(Variable *t, int v, int a, int b, int c) {
    if(t->containsValue(v)) {
        ry[b]  = a;
        rzx[c] = a;
        rzy[c] = b;
        return true;
    }
    return false;
}

bool DistXYeqZ::supportZ(Variable *t, int v, int a, int b, int c) {
    if(t->containsValue(v)) {
        rzx[c] = a;
        rzy[c] = b;
        return true;
    }
    return false;
}


bool DistXYeqZ::filter(Variable *dummy) {
    for(int idx : x->domain) {
        int vx = x->domain.toVal(idx);
        if(y->containsIdv(rx[idx]) && z->containsValue(abs(vx - y->domain.toVal(rx[idx]))))
            continue;
        if(y->size() <= z->size())
            for(int idy : y->domain) {
                int vz = abs(vx - y->domain.toVal(idy));
                if(supportX(z, vz, idx, idy, z->domain.toIdv(vz)))
                    goto stopLoop1;
            }
        else
            for(int idz : z->domain) {
                int vy = vx - z->domain.toVal(idz);
                if(supportX(y, vy, idx, y->domain.toIdv(vy), idz))
                    goto stopLoop1;
                vy = vx + z->domain.toVal(idz);
                if(supportX(y, vy, idx, y->domain.toIdv(vy), idz))
                    goto stopLoop1;
            }
        if(solver->delIdv(x, idx) == false)
            return false;
        stopLoop1:;
    }



    for(int idy : y->domain) {
        int vy = y->domain.toVal(idy);
        if(x->containsIdv(ry[idy]) && z->containsValue(abs(vy - x->domain.toVal(ry[idy]))))
            continue;
        if(x->size() <= z->size())
            for(int idx : x->domain) {
                int vz = abs(vy - x->domain.toVal(idx));
                if(supportY(z, vz, idx, idy, z->domain.toIdv(vz)))
                    goto stopLoop2;
            }
        else
            for(int idz : z->domain) {
                int vx = vy - z->domain.toVal(idz);
                if(supportY(x, vx, x->domain.toIdv(vx), idy, idz))
                    goto stopLoop2;
                vx = vy + z->domain.toVal(idz);
                if(supportY(x, vx, x->domain.toIdv(vx), idy, idz))
                    goto stopLoop2;
            }
        if(solver->delIdv(y, idy) == false)
            return false;
        stopLoop2:;
    }



    for(int idz : z->domain) {
        int vz = z->domain.toVal(idz);
        if(rzx[idz] != -1 && rzx[idz] < x->domain.maxSize() && x->containsIdv(rzx[idz])
           && rzy[idz] < y->domain.maxSize() && y->containsIdv(rzy[idz]))
            continue;
        if(x->size() <= y->size())
            for(int idx : x->domain) {
                int vy = x->domain.toVal(idx) - vz;
                if(supportZ(y, vy, idx, y->domain.toIdv(vy), idz))
                    goto stopLoop3;
                vy = x->domain.toVal(idx) + vz;
                if(supportZ(y, vy, idx, y->domain.toIdv(vy), idz))
                    goto stopLoop3;
            }
        else
            for(int idy : y->domain) {
                int vx = y->domain.toVal(idy) - vz;
                if(supportZ(x, vx, x->domain.toIdv(vx), idy, idz))
                    goto stopLoop3;
                vx = y->domain.toVal(idy) + vz;
                if(supportZ(x, vx, y->domain.toIdv(vx), idy, idz))
                    goto stopLoop3;
            }
        if(solver->delIdv(z, idz) == false)
            return false;
        stopLoop3:;
    }

    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

DistXYeqZ::DistXYeqZ(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz) : Ternary(p, n, xx, yy, zz) {
    type = "|X - Y| = Z";
    rx.growTo(x->domain.maxSize());
    ry.growTo(y->domain.maxSize());

    rzx.growTo(z->domain.maxSize(), -1);
    rzy.growTo(z->domain.maxSize(), -1);
}
