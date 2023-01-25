//
// Created by audemard on 23/01/23.
//

#ifndef COSOCO_CUMULATIVEHEIGHTANDWIDTHSVARIABLES_H
#define COSOCO_CUMULATIVEHEIGHTANDWIDTHSVARIABLES_H
#include "Cumulative.h"

namespace Cosoco {
class CumulativeHeightAndWidthsVariables : public Cumulative {
   protected:
    vec<Variable *> widthVariables, heightVariables;

   public:
    CumulativeHeightAndWidthsVariables(Problem &p, std::string n, vec<Variable *> &vars, vec<Variable *> &, vec<Variable *> &,
                                       int limit);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;
    int  maxWidth(int posx);
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

}   // namespace Cosoco
#endif   // COSOCO_CUMULATIVEHEIGHTANDWIDTHSVARIABLES_H
