#include "SumScalarLEVar.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool SumScalarLEVar::isSatisfiedBy(vec<int> &tuple) { return sum(tuple) <= tuple.last(); }

bool SumScalarLEVar::isCorrectlyDefined() {
    SumScalar::isCorrectlyDefined();

    if(scope.size() != half * 2 + 1)
        throw std::logic_error("Constraint " + std::to_string(idc) + ": Sum Scalar Var, list and coefficients mus have same size");
    return true;
}

//----------------------------------------------
// Filtering
//----------------------------------------------


bool SumScalarLEVar::filter(Variable *x) {
    recomputeBounds();
    if(solver->delValuesLowerOrEqualThan(limit, min - 1) == false)
        return false;

    if(max <= limit->maximum())
        return true;
    if(min == limit->maximum()) {   // this is the only case where we can filter
        assert(limit->size() == 1);
        removeFrom01vs1(1);
    }

    return true;
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------


SumScalarLEVar::SumScalarLEVar(Problem &p, std::string n, vec<Variable *> &variables, vec<Variable *> &coefs, Variable *_z)
    : SumScalar(p, n, variables.size() * 2 + 1), limit(_z) {
    vec<Variable *> vars;
    variables.copyTo(vars);
    for(Variable *x : coefs) vars.push(x);
    vars.push(_z);
    scopeInitialisation(vars);
    half = variables.size();
    type = "SumScalarBooleanLEVar";
}
