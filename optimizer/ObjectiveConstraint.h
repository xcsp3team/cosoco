#ifndef OBJECTIVECONSTRAINT_H
#define OBJECTIVECONSTRAINT_H

#include "mtl/Vec.h"

namespace Cosoco {
class ObjectiveConstraint {   // This constraint can be used as an objective
   public:
    virtual ~ObjectiveConstraint()                = default;
    virtual void updateBound(long bound)          = 0;   // Update the current bound
    virtual long maxUpperBound()                  = 0;   // Bounds are included
    virtual long minLowerBound()                  = 0;   // Bounds are included
    virtual long computeScore(vec<int> &solution) = 0;   // Compute the current score of the constraint
};
}   // namespace Cosoco


#endif /* OBJECTIVECONSTRAINT_H */
