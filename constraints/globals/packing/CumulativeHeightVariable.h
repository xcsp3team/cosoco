//
// Created by audemard on 21/01/23.
//

#ifndef COSOCO_CUMULATIVEHEIGHTVARIABLE_H
#define COSOCO_CUMULATIVEHEIGHTVARIABLE_H

#include "Cumulative.h"
namespace Cosoco {
class CumulativeHeightVariable : public Cumulative {
   protected:
    vec<Variable *> heightVariables;

   public:
    CumulativeHeightVariable(Problem &p, std::string n, vec<Variable *> &vars, vec<Variable *> &scope, vec<int> &,
                             vec<Variable *> &, int limit);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco
#endif   // COSOCO_CUMULATIVEHEIGHTVARIABLE_H
