#ifndef COSOCO_MAXIMUMVARIABLELE_H
#define COSOCO_MAXIMUMVARIABLELE_H

#include "MaximumVariable.h"

namespace Cosoco {
class MaximumVariableLE : public MaximumVariable {
    MaximumVariableLE(Problem &p, std::string n, vec<Variable *> &vars, Variable *v) : MaximumVariable(p, n, vars, v) { }


    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif   // COSOCO_MAXIMUMVARIABLELE_H
