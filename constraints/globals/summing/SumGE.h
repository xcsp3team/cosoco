#ifndef SUMGE_H
#define SUMGE_H

#include "constraints/globals/summing/Sum.h"
#include "optimizer/ObjectiveConstraint.h"
namespace Cosoco {

class SumGE : public Sum, public ObjectiveConstraint {
   protected:
    long min, max;
    int  leftmostPositiveCoefficientPosition;
    void computeBounds();

   public:
    SumGE(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &coefs, long l);
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
