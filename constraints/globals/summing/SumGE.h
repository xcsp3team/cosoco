#ifndef SUMGE_H
#define SUMGE_H

#include "Sum.h"
namespace Cosoco {


class SumGE : public SimpleSum, public ObjectiveConstraint {
   protected:
    bool filter(Variable *x) override;

   public:
    SumGE(Problem &p, std::string n, vec<Variable *> &vars, long l) : SimpleSum(p, std::move(n), vars, l) { type = "SumGE"; }
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
    // Functions related to Objective constraint
    void updateBound(long bound) override;            // Update the current bound
    long maxUpperBound() override;                    // Bounds are included
    long minLowerBound() override;                    // Bounds are included
    long computeScore(vec<int> &solution) override;   // Compute the current score of the constraint
};

class WeightedSumGE : public WeightedSum, public ObjectiveConstraint {
   protected:
    int leftmostPositiveCoefficientPosition;

    bool filter(Variable *x) override;

   public:
    WeightedSumGE(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &coefs, long l);


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
