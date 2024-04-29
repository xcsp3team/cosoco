//
// Created by audemard on 09/04/24.
//

#include "Reification.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool ReifLE::isSatisfiedBy(vec<int> &tuple) { return (tuple[0] == 1) == (tuple[1] <= tuple[2]); }
bool ReifLT::isSatisfiedBy(vec<int> &tuple) { return (tuple[0] == 1) == (tuple[1] < tuple[2]); }
bool ReifEQ::isSatisfiedBy(vec<int> &tuple) { return (tuple[0] == 1) == (tuple[1] == tuple[2]); }
bool ReifNE::isSatisfiedBy(vec<int> &tuple) { return (tuple[0] == 1) == (tuple[1] != tuple[2]); }

bool XeqYeqK::isSatisfiedBy(vec<int> &tuple) { return tuple[0] == (tuple[1] == k); }
bool XeqYneK::isSatisfiedBy(vec<int> &tuple) { return tuple[0] == (tuple[1] != k); }
bool XeqKleY::isSatisfiedBy(vec<int> &tuple) { return tuple[0] == (k <= tuple[1]); }
bool XeqYleK::isSatisfiedBy(vec<int> &tuple) { return tuple[0] == (tuple[1] <= k); }
bool XeqAndY::isSatisfiedBy(vec<int> &tuple) {
    if(tuple.last() == 0) {
        for(int i = 0; i < tuple.size() - 1; i++)
            if(tuple[i] == 0)
                return true;
        return false;
    }
    for(int i = 0; i < tuple.size() - 1; i++)
        if(tuple[i] == 0)
            return false;
    return true;
}


//----------------------------------------------
// Filtering
//----------------------------------------------


bool ReifLE::filter(Variable *dummy) {
    if(x->domain.size() == 1 && x->value() == 0) {   // Assigned at 0  => y > z
        if(solver->isAssigned(y) == false)
            if(solver->delValuesLowerOrEqualThan(y, z->minimum()) == false)
                return false;

        if(solver->isAssigned(z) == false)
            if(solver->delValuesGreaterOrEqualThan(z, y->maximum()) == false)
                return false;
        return true;
    }
    if(x->domain.size() == 1 && x->value() == 1) {   // Assigned at 0  => y > z
        if(solver->isAssigned(y) == false)
            if(solver->delValuesGreaterOrEqualThan(y, z->maximum() + 1) == false)
                return false;

        if(solver->isAssigned(z) == false)
            if(solver->delValuesLowerOrEqualThan(z, y->minimum() - 1) == false)
                return false;
        return true;
    }
    if(y->maximum() <= z->minimum())   // for sure x = 1
        return solver->assignToVal(x, 1);

    if(y->minimum() > z->maximum())   // for sure x = 0
        return solver->assignToVal(x, 0);
    return true;
}


bool ReifLT::filter(Variable *dummy) {
    if(x->domain.size() == 1 && x->value() == 0) {   // Assigned at 0  => y > z
        if(solver->isAssigned(y) == false)
            if(solver->delValuesLowerOrEqualThan(y, z->minimum() - 1) == false)
                return false;

        if(solver->isAssigned(z) == false)
            if(solver->delValuesGreaterOrEqualThan(z, y->maximum() + 1) == false)
                return false;
        return true;
    }
    if(x->domain.size() == 1 && x->value() == 1) {   // Assigned at 0  => y > z
        if(solver->isAssigned(y) == false)
            if(solver->delValuesGreaterOrEqualThan(y, z->maximum()) == false)
                return false;

        if(solver->isAssigned(z) == false)
            if(solver->delValuesLowerOrEqualThan(z, y->minimum()) == false)
                return false;
        return true;
    }
    if(y->maximum() < z->minimum())   // for sure x = 1
        return solver->assignToVal(x, 1);

    if(y->minimum() >= z->maximum())   // for sure x = 0
        return solver->assignToVal(x, 0);
    return true;
}

bool ReifEQ::filter(Cosoco::Variable *dummy) {
    if(y->size() == 1 && z->size() == 1)
        return solver->assignToVal(x, y->value() == z->value() ? 1 : 0);


    if(x->domain.size() == 1 && x->value() == 0) {   // Assigned at 0  => y != z
        if(y->size() == 1)
            return solver->delVal(z, y->value());
        if(z->size() == 1)
            return solver->delVal(y, z->value());
    }


    if(x->domain.size() == 1 && x->value() == 1) {   // Assigned at 1  => y = z
        for(int idv : y->domain)
            if(z->containsValue(y->domain.toVal(idv)) == false && solver->delIdv(y, idv) == false)
                return false;
        for(int idv : z->domain)
            if(y->containsValue(z->domain.toVal(idv)) == false && solver->delIdv(z, idv) == false)
                return false;
    }
    // support for x = 1 ?
    if(residue != INT_MAX && y->containsValue(residue) && z->containsValue(residue))
        return true;

    residue = INT_MAX;   // look for a residue
    for(int idv : y->domain)
        if(z->containsValue(y->domain.toVal(idv))) {
            residue = y->domain.toVal(idv);
            break;
        }
    if(residue == INT_MAX && solver->delVal(x, 1) == false)
        return false;
    return true;
}


bool ReifNE::filter(Cosoco::Variable *dummy) {
    if(y->size() == 1 && z->size() == 1)
        return solver->assignToVal(x, y->value() != z->value() ? 1 : 0);


    if(x->domain.size() == 1 && x->value() == 1) {   // Assigned at 1  => y != z
        if(y->size() == 1)
            return solver->delVal(z, y->value());
        if(z->size() == 1)
            return solver->delVal(y, z->value());
    }


    if(x->domain.size() == 1 && x->value() == 0) {   // Assigned at 0  => y = z
        for(int idv : y->domain)
            if(z->containsValue(y->domain.toVal(idv)) == false && solver->delIdv(y, idv) == false)
                return false;
        for(int idv : z->domain)
            if(y->containsValue(z->domain.toVal(idv)) == false && solver->delIdv(z, idv) == false)
                return false;
    }
    // support for x = 0 ?
    if(residue != INT_MAX && y->containsValue(residue) && z->containsValue(residue))
        return true;

    residue = INT_MAX;   // look for a residue
    for(int idv : y->domain)
        if(z->containsValue(y->domain.toVal(idv))) {
            residue = y->domain.toVal(idv);
            break;
        }
    if(residue == INT_MAX && solver->delVal(x, 0) == false)
        return false;
    return true;
}

bool XeqYeqK::filter(Variable *dummy) {
    if(x->size() == 1) {
        if(x->value() == 0) {
            if(solver->delVal(y, k) == false)
                return false;
            solver->entail(this);
            return true;
        }
        if(solver->assignToVal(y, k) == false)
            return false;
        solver->entail(this);
        return true;
    }
    if(y->containsValue(k) == false) {
        if(solver->assignToVal(x, 0) == false)
            return false;
        solver->entail(this);
        return true;
    }

    if(y->size() == 1) {
        if(solver->assignToVal(x, y->value() == k) == false)
            return false;
        solver->entail(this);
        return true;
    }
    return true;
}


bool XeqYneK::filter(Variable *dummy) {
    if(x->size() == 1) {
        if(x->value() == 0) {   // y = k
            if(solver->assignToVal(y, k) == false)
                return false;
            solver->entail(this);
            return true;
        }
        if(solver->delVal(y, k) == false)   // y != k
            return false;
        solver->entail(this);
        return true;
    }
    if(y->containsValue(k) == false) {
        if(solver->assignToVal(x, 1) == false)
            return false;
        solver->entail(this);
        return true;
    }

    if(y->size() == 1) {
        if(solver->assignToVal(x, y->value() != k) == false)
            return false;
        solver->entail(this);
        return true;
    }
    return true;
}


bool XeqKleY::filter(Variable *dummy) {
    if(x->size() == 1) {
        if(x->value() == 0) {   // y < k
            if(solver->delValuesGreaterOrEqualThan(y, k) == false)
                return false;
            solver->entail(this);
            return true;
        }
        // y >= k

        if(solver->delValuesLowerOrEqualThan(y, k - 1) == false)
            return false;
        solver->entail(this);
        return true;
    }

    if(y->minimum() >= k) {
        solver->assignToVal(x, 1);
        solver->entail(this);
        return true;
    }

    if(y->maximum() < k) {
        solver->assignToVal(x, 0);
        solver->entail(this);
        return true;
    }
    return true;
}


bool XeqYleK::filter(Variable *dummy) {
    if(x->size() == 1) {
        if(x->value() == 0) {   // y > k
            if(solver->delValuesLowerOrEqualThan(y, k) == false)
                return false;
            solver->entail(this);
            return true;
        }
        // y <= k
        if(solver->delValuesGreaterOrEqualThan(y, k + 1) == false)
            return false;
        solver->entail(this);
        return true;
    }

    if(y->maximum() <= k) {
        solver->assignToVal(x, 1);
        solver->entail(this);
        return true;
    }

    if(y->minimum() > k) {
        solver->assignToVal(x, 0);
        solver->entail(this);
        return true;
    }
    return true;
}

Variable *XeqAndY::findSentinel(Cosoco::Variable *other) {
    for(Variable *y : list)
        if(y != other && y->minimum() == 0)
            return y;
    return nullptr;
}

bool XeqAndY::filter(Cosoco::Variable *dummy) {
    for(Variable *y : list)
        if(y->maximum() == 0) {                      // at least one var is false
            if(solver->assignToVal(x, 0) == false)   // so x is false
                return false;
            solver->entail(this);
            return true;
        }

    if(x->minimum() == 1) {   // x is true, all are true
        for(Variable *y : list) solver->assignToVal(y, 1);
        solver->entail(this);
        return true;
    }

    if(x->maximum() == 0) {        // At least 1 y must be false
        if(s1->minimum() == 1) {   // this is not a sentinel
            Variable *tmp = findSentinel(s2);
            if(tmp == nullptr) {   // no other sentinel
                if(solver->assignToVal(s2, 0) == false)
                    return false;
                solver->entail(this);
                return true;
            }
            s1 = tmp;
        }
        if(s2->minimum() == 1) {   // this is not a sentinel
            Variable *tmp = findSentinel(s1);
            if(tmp == nullptr) {   // no other sentinel
                if(solver->assignToVal(s1, 0) == false)
                    return false;
                solver->entail(this);
                return true;
            }
            s2 = tmp;
        }
        return true;
    }

    if(s1->minimum() == 0 || s2->minimum() == 0)
        return true;   // everything is possible
    for(Variable *y : list)
        if(y->minimum() == 0) {
            s1 = y;
            return true;
        }
    solver->delVal(x, 0);
    solver->entail(this);
    return true;
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

ReifLE::ReifLE(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz) : Ternary(p, n, xx, yy, zz) {
    assert(xx->domain.maxSize() == 2);
    type = "X = (Y <= Z)";
}

ReifLT::ReifLT(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz) : Ternary(p, n, xx, yy, zz) {
    assert(xx->domain.maxSize() == 2);
    type = "X = (Y < Z)";
}

ReifEQ::ReifEQ(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz) : Ternary(p, n, xx, yy, zz) {
    residue = INT_MAX;
    assert(xx->domain.maxSize() == 2);
    type = "X = (Y = Z)";
}

ReifNE::ReifNE(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz) : Ternary(p, n, xx, yy, zz) {
    residue = INT_MAX;
    assert(xx->domain.maxSize() == 2);
    type = "X = (Y != Z)";
}

XeqYeqK::XeqYeqK(Problem &p, std::string n, Variable *xx, Variable *yy, int _k) : Binary(p, n, xx, yy), k(_k) {
    type = "X = (Y = k)";
}

XeqYneK::XeqYneK(Problem &p, std::string n, Variable *xx, Variable *yy, int _k) : Binary(p, n, xx, yy), k(_k) {
    type = "X = (Y != k)";
}

XeqKleY::XeqKleY(Problem &p, std::string n, Variable *xx, Variable *yy, int _k) : Binary(p, n, xx, yy), k(_k) {
    type = "X = (k <= Y)";
}
XeqYleK::XeqYleK(Problem &p, std::string n, Variable *xx, Variable *yy, int _k) : Binary(p, n, xx, yy), k(_k) {
    type = "X = (Y <= k)";
}

XeqAndY::XeqAndY(Cosoco::Problem &p, std::string n, vec<Cosoco::Variable *> &vars) : Constraint(p, n, vars) {
    type = "X = AND(Y_i)";
    x    = vars.last();
    vars.copyTo(list);
    list.pop();   // x is the last var
    s1 = list[0];
    s2 = list[1];
}