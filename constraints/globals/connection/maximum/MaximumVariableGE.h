#ifndef COSOCO_MAXIMUMVARIABLEGE_H
#define COSOCO_MAXIMUMVARIABLEGE_H

#include "MaximumVariable.h"

namespace Cosoco {
class MaximumVariableGE : public MaximumVariable {
    MaximumVariableGE(Problem &p, std::string n, vec<Variable *> &vars, Variable *v) : MaximumVariable(p, n, vars, v) { }


    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif   // COSOCO_MAXIMUMVARIABLEGE_H
