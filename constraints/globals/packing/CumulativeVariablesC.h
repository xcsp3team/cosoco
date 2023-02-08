//
// Created by audemard on 21/12/2022.
//

#ifndef COSOCO_CUMULATIVEVARIABLESC_H
#define COSOCO_CUMULATIVEVARIABLESC_H


#include "Cumulative.h"
namespace Cosoco {

class CumulativeVariablesC : public Cumulative {
   protected:
    Variable *limitVariable;

   public:
    CumulativeVariablesC(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &, vec<int> &, Variable *limit);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco

#endif   // COSOCO_CUMULATIVEVARIABLESC_H
