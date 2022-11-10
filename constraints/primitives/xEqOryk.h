//
// Created by audemard on 29/01/2021.
//

#ifndef COSOCO_XEQORYK_H
#define COSOCO_XEQORYK_H

#include "constraints/globals/GlobalConstraint.h"
namespace Cosoco {
class xEqOryk : public GlobalConstraint {
    Variable *result;
    vec<Variable *>clause;
    vec<int> values;
   public:
    xEqOryk(Problem &p, std::string n, Variable *r, vec<Variable *> &vars, vec<int>&_values);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif   // COSOCO_XEQORYK_H
