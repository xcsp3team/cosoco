//
// Created by audemard on 23/01/23.
//

#ifndef COSOCO_CUMULATIVEVARIABLESW_H
#define COSOCO_CUMULATIVEVARIABLESW_H

#include "Cumulative.h"
namespace Cosoco {
class CumulativeVariablesW : public Cumulative {
   protected:
    vec<Variable *> widthVariables;

   public:
    CumulativeVariablesW(Problem &p, std::string n, vec<Variable *> &vars, vec<Variable *> &, vec<int> &, int limit);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;
    int  maxWidth(int posx) override;
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif   // COSOCO_CUMULATIVEVARIABLESW_H
