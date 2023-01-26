//
// Created by audemard on 21/12/2022.
//

#ifndef COSOCO_CUMULATIVECONDITIONVARIABLE_H
#define COSOCO_CUMULATIVECONDITIONVARIABLE_H


#include "Cumulative.h"
namespace Cosoco {

class CumulativeConditionVariable : public Cumulative {
   protected:
    Variable *limitVariable;

   public:
    CumulativeConditionVariable(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &, vec<int> &, Variable *limit);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;
    void filterLimitVariable(Variable *x);

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco

#endif   // COSOCO_CUMULATIVECONDITIONVARIABLE_H
