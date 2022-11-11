//
// Created by audemard on 26/04/2018.
//

#ifndef COSOCO_NVALUESLEK_H
#define COSOCO_NVALUESLEK_H


#include <constraints/globals/GlobalConstraint.h>

#include <set>
namespace Cosoco {
class NValuesLEK : public GlobalConstraint, public ObjectiveConstraint {
   public:
    int           k;
    std::set<int> myset;
    NValuesLEK(Problem &p, std::string n, vec<Variable *> &vars, int k);

    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool isCorrectlyDefined() override;
    int  countDistinct(vec<int> &tuple);

    // Functions related to Objective constraint
    void updateBound(long bound) override;            // Update the current bound
    long maxUpperBound() override;                    // Bounds are included
    long minLowerBound() override;                    // Bounds are included
    long computeScore(vec<int> &solution) override;   // Compute the current score of the constraint
};

}   // namespace Cosoco
#endif   // COSOCO_NVALUESLEK_H
