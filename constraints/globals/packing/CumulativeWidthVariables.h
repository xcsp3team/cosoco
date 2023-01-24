//
// Created by audemard on 23/01/23.
//

#ifndef COSOCO_CUMULATIVEWIDTHVARIABLES_H
#define COSOCO_CUMULATIVEWIDTHVARIABLES_H

#include "Cumulative.h"
namespace Cosoco {
class CumulativeWidthVariables : public Cumulative {
   protected:
    vec<Variable *> widthVariables;

   public:
    CumulativeWidthVariables(Problem &p, std::string n, vec<Variable *> &vars, vec<Variable *> &scope, vec<Variable *> &,
                             vec<int> &, int limit);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;
    int  maxWidth(int posx) override;
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif   // COSOCO_CUMULATIVEWIDTHVARIABLES_H
