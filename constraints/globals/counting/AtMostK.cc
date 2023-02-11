#include "AtMostK.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool AtMostK::isSatisfiedBy(vec<int> &tuple) {
    int nb = 0;
    for(int v : tuple)
        if(v == value)
            nb++;
    return nb <= k;
}


bool AtMostK::isCorrectlyDefined() {
    std::cout << k << " " << scope.size() << std::endl;
    if(k < 1)
        throw std::logic_error("Constraint " + std::to_string(idc) + ": AtMost must have 1<=k <= size(list)");
    for(Variable *x : scope)
        if(x->containsValue(value) == false)
            throw std::logic_error("Constraint " + std::to_string(idc) + ": AtMost, all variables must contain value");
    return true;
}

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool AtMostK::filter(Variable *x) {
    if(k >= scope.size()) {
        return true;
    }
    if(x->isUniqueValue(value) == false)
        return true;

    int cnt = 0;
    for(Variable *y : scope) {
        if(y->isUniqueValue(value) && ++cnt > k) {
            return false;   // Too many variables have this value
        }
    }

    if(cnt == k)   // others variables can not be equal to k
        for(Variable *y : scope) {
            if(y->isUniqueValue(value) == false && solver->delVal(y, value) == false)
                return false;
        }
    return true;
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

AtMostK::AtMostK(Problem &p, std::string n, vec<Variable *> &vars, int kk, int val)
    : GlobalConstraint(p, n, "At most", vars), k(kk), value(val) { }
