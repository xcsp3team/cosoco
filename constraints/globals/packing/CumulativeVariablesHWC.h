//
// Created by audemard on 08/02/23.
//

#ifndef COSOCO_CUMULATIVEVARIABLESHWC_H
#define COSOCO_CUMULATIVEVARIABLESHWC_H

#include "Cumulative.h"
namespace Cosoco {
class CumulativeVariablesHWC : public Cumulative {
    vec<Variable *> widthVariables, heightVariables;
    Variable *limitVariable;

   public :
    CumulativeVariablesHWC(Problem &p, std::string n, vec<Variable *> &vars, vec<Variable *> &, vec<Variable *> &,
                           Variable *limit);
    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;
    int  maxWidth(int posx) override;
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco

#endif   // COSOCO_CUMULATIVEVARIABLESHWC_H
