//
// Created by audemard on 23/01/23.
//

#ifndef COSOCO_CUMULATIVEVARIABLESHW_H
#define COSOCO_CUMULATIVEVARIABLESHW_H
#include "Cumulative.h"

namespace Cosoco {
class CumulativeVariablesHW : public Cumulative {
   protected:
    vec<Variable *> widthVariables, heightVariables;

   public:
    CumulativeVariablesHW(Problem &p, std::string n, vec<Variable *> &vars, vec<Variable *> &, vec<Variable *> &, int limit);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;
    int  maxWidth(int posx) override;
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

}   // namespace Cosoco
#endif   // COSOCO_CUMULATIVEVARIABLESHW_H
