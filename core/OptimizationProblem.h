#ifndef COSOCO_PROBLEMOPTIMIZATION_H
#define COSOCO_PROBLEMOPTIMIZATION_H

#include "Problem.h"
#include "optimizer/ObjectiveConstraint.h"

namespace Cosoco {
enum OptimisationType { Minimize, Maximize };


class OptimizationProblem : public Problem {   // An optimosation problem is a probem containing at least one
   public:                                     // objectiveLB or objectiveUB
    explicit OptimizationProblem(const std::string &n);

    ObjectiveConstraint *objectiveLB;   // ctr >= ct
    ObjectiveConstraint *objectiveUB;   // ctr <= ct
    OptimisationType     type;          // Minimize or Maximize?

    void addObjectiveLB(ObjectiveConstraint *o);   // Add the already added LB constraint
    void addObjectiveUB(ObjectiveConstraint *o);   // Add the already added UB constraint
};
}   // namespace Cosoco


#endif   // COSOCO_PROBLEMOPTIMIZATION_H
