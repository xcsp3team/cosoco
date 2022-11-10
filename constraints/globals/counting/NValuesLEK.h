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

    virtual bool filter(Variable *x) override;

    // Checking
    virtual bool isSatisfiedBy(vec<int> &tuple) override;
    virtual bool isCorrectlyDefined() override;
    int          countDistinct(vec<int> &tuple);

    // Functions related to Objective constraint
    virtual void updateBound(long bound) override;            // Update the current bound
    virtual long maxUpperBound() override;                    // Bounds are included
    virtual long minLowerBound() override;                    // Bounds are included
    virtual long computeScore(vec<int> &solution) override;   // Compute the current score of the constraint
};

}   // namespace Cosoco
#endif   // COSOCO_NVALUESLEK_H
