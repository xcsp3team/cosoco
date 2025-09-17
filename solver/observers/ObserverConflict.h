
#ifndef OBSERVERCONFLICT_H
#define OBSERVERCONFLICT_H
#include "constraints/Constraint.h"

namespace Cosoco {
class ObserverConflict {
   public:
    virtual ~ObserverConflict()                           = default;
    virtual void notifyConflict(Constraint *c, int level) = 0;
};
}   // namespace Cosoco

#endif /* OBSERVERSCONFLICT_H */
