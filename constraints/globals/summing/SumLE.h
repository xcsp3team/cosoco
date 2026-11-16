//
// Created by audemard on 23/10/2026.
//

#ifndef COSOCO_SUMLE_H
#define COSOCO_SUMLE_H


#include "Sum.h"
namespace Cosoco {
class SumSimpleLE : public SimpleSum, public ObjectiveConstraint {
   public:
    SumSimpleLE(Problem &p, vec<Variable *> &vars, long l) : SimpleSum(p, vars, l) { type = "SumLE"; }
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;

    bool filter(Variable *x) override;

    // Functions related to Objective constraint
    void updateBound(long bound) override;            // Update the current bound
    long maxUpperBound() override;                    // Bounds are included
    long minLowerBound() override;                    // Bounds are included
    long computeScore(vec<int> &solution) override;   // Compute the current score of the constraint
};
}   // namespace Cosoco

#endif   // COSOCO_SUMLE_H
