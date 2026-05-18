#ifndef COSOCO_PROBLEMOPTIMIZATION_H
#define COSOCO_PROBLEMOPTIMIZATION_H

#include "Problem.h"
#include "optimizer/ObjectiveConstraint.h"

namespace Cosoco {
enum OptimisationType { Minimize, Maximize };


class OptimizationProblem : public Problem {   // An optimization problem is a problem containing at least one
   public:                                     // objectiveLB or objectiveUB
    explicit OptimizationProblem(const std::string &n);

    ObjectiveConstraint *objectiveLB;   // ctr >= ct
    ObjectiveConstraint *objectiveUB;   // ctr <= ct
    OptimisationType     type;          // Minimize or Maximize?

    void addObjectiveLB(ObjectiveConstraint *o, bool alreadyAdded = false);   // Add the LB constraint
    void addObjectiveUB(ObjectiveConstraint *o, bool alreadyAdded = false);   // Add the UB constraint
};
}   // namespace Cosoco


#endif   // COSOCO_PROBLEMOPTIMIZATION_H
