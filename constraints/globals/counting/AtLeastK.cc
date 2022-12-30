#include "AtLeastK.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool AtLeastK::isSatisfiedBy(vec<int> &tuple) {
    int nb = 0;
    for(int v : tuple)
        if(v == value)
            nb++;
    return nb >= k;
}


bool AtLeastK::isCorrectlyDefined() {
    if(k < 1 || k > scope.size())
        throw std::logic_error("Constraint " + std::to_string(idc) +
                               ": Atleast must have 1<=k <= size(list) (k=" + std::to_string(k) + ")");
    for(Variable *x : scope)
        if(x->containsValue(value) == false)
            throw std::logic_error("Constraint " + std::to_string(idc) + ": AtLeast, all variables must contain value");
    return true;
}


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool AtLeastK::filter(Variable *x) {
    int posIdx = idxToScopePosition[x->idx];
    assert(posIdx != NOTINSCOPE);


    if(sentinels.contains(posIdx) == false)   // this variable is not a sentinel
        return true;
    if(x->containsValue(value) == true)   // No problem, this variable helps the constraint
        return true;

    for(int i = sentinels.size(); i < sentinels.maxSize(); i++) {   // Check for a new sentinel on remaining variables
        int newposIdx = sentinels[i];
        if(scope[newposIdx]->containsValue(value)) {   // We have one
            sentinels.del(posIdx);
            sentinels.add(newposIdx);
            return true;
        }
    }

    // No new sentinels found, need to assign all k remaining variables to value
    for(int posIdx2 : reverse(sentinels))
        if(posIdx2 != posIdx && solver->assignToVal(scope[posIdx2], value) == false)
            return false;
    solver->entail(this);
    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

AtLeastK::AtLeastK(Problem &p, std::string n, vec<Variable *> &vars, int kk, int val)
    : GlobalConstraint(p, n, "At least", vars), k(kk), value(val), sentinels(vars.size(), false) {
    // Need k+1 sentinels
    for(int i = 0; i < k + 1; i++) sentinels.add(i);
}
