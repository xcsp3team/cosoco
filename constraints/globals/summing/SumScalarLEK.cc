#include "SumScalarLEK.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool SumScalarLEK::isSatisfiedBy(vec<int> &tuple) {return sum(tuple) <= limit; }


bool SumScalarLEK::isCorrectlyDefined() {
    SumScalar::isCorrectlyDefined();
    if(limit < 0 || limit > half)
        throw std::logic_error("Constraint " + std::to_string(idc) + ": Sum Scalar, limit is not ok");
    if(scope.size() != half * 2)
        throw std::logic_error("Constraint " + std::to_string(idc) +
                               ": Sum Scalar, list and coefficients mus have same size");
    return true;
}
//----------------------------------------------
// Filtering
//----------------------------------------------


bool SumScalarLEK::filter(Variable *x) {
    recomputeBounds();
    if (max <= limit) {
        solver->entail(this);
        return true;
    }
    if (min > limit)
        return false;
    if (min == limit) // this is the only case where we can filter
        removeFrom01vs1(1);
    return true;
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------


SumScalarLEK::SumScalarLEK(Problem &p, std::string n, vec<Variable *> &variables, vec<Variable *> &coefs, long l)
    : SumScalar(p, n, variables.size()*2), limit(l) {
    vec<Variable *> vars;
    variables.copyTo(vars);
    for(Variable *x : coefs) vars.push(x);
    scopeInitialisation(vars);
    half = variables.size();
}
