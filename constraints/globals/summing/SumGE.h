#ifndef SUMGE_H
#define SUMGE_H

#include "Sum.h"
namespace Cosoco {

class SumGE : public SumIncremental, public ObjectiveConstraint {
   protected:
    int leftmostPositiveCoefficientPosition;

   public:
    SumGE(Problem &p, vec<Variable *> &vars, vec<int> &coefs, long l);
    bool filter(Variable *x) override;


    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;

    // Functions related to Objective constraint
    void updateBound(long bound) override;            // Update the current bound
    long maxUpperBound() override;                    // Bounds are included
    long minLowerBound() override;                    // Bounds are included
    long computeScore(vec<int> &solution) override;   // Compute the current score of the constraint
};
}   // namespace Cosoco


#endif /* SUMGE_H */
