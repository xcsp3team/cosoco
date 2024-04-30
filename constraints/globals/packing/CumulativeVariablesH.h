//
// Created by audemard on 21/01/23.
//

#ifndef COSOCO_CUMULATIVEVARIABLESH_H
#define COSOCO_CUMULATIVEVARIABLESH_H

#include "Cumulative.h"
namespace Cosoco {
class CumulativeVariablesH : public Cumulative {
   protected:
    vec<Variable *> heightVariables;

   public:
    CumulativeVariablesH(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &, vec<Variable *> &, int limit);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

class CumulativeVariablesHLimitV : public CumulativeVariablesH {
   protected:
    Variable *limitvar;

   public:
    CumulativeVariablesHLimitV(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &, vec<Variable *> &, Variable *limit);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};


}   // namespace Cosoco
#endif   // COSOCO_CUMULATIVEVARIABLESH_H
