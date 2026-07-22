//
// Created by audemard on 22/07/2026.
//

#ifndef COSOCO_SUMLE_H
#define COSOCO_SUMLE_H
#include "Sum.h"

namespace Cosoco {
class SumLE : public SimpleSum, public ObjectiveConstraint {
   protected:
    bool filter(Variable *x) override;

   public:
    SumLE(Problem &p, std::string n, vec<Variable *> &vars, long l) : SimpleSum(p, std::move(n), vars, l) { type = "SumLE"; }
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;

    // Functions related to Objective constraint
    void updateBound(long bound) override;            // Update the current bound
    long maxUpperBound() override;                    // Bounds are included
    long minLowerBound() override;                    // Bounds are included
    long computeScore(vec<int> &solution) override;   // Compute the current score of the constraint
};

}   // namespace Cosoco


#endif   // COSOCO_SUMLE_H
