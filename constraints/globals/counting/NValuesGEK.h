//
// Created by audemard on 26/04/2018.
//

#ifndef COSOCO_NVALUESGEK_H
#define COSOCO_NVALUESGEK_H


#include <constraints/globals/GlobalConstraint.h>

#include <set>
namespace Cosoco {
class NValuesGEK : public GlobalConstraint, public ObjectiveConstraint {
   public:
    unsigned int  k;
    std::set<int> fixedValues;
    std::set<int> unfixedVariables;
    NValuesGEK(Problem &p, std::string n, vec<Variable *> &vars, int k);

    bool filter(Variable *x) override;
    void initializeSets();
    // Checking
    bool         isSatisfiedBy(vec<int> &tuple) override;
    bool         isCorrectlyDefined() override;
    unsigned int countDistinct(vec<int> &tuple);

    // Functions related to Objective constraint
    void updateBound(long bound) override;            // Update the current bound
    long maxUpperBound() override;                    // Bounds are included
    long minLowerBound() override;                    // Bounds are included
    long computeScore(vec<int> &solution) override;   // Compute the current score of the constraint
};

}   // namespace Cosoco
#endif   // COSOCO_NVALUESLEK_H
