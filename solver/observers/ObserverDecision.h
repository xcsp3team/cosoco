#ifndef OBERSERDECISION_H
#define OBERSERDECISION_H
#include "core/Variable.h"
namespace Cosoco {
class Solver;

class ObserverNewDecision {
   public:
    virtual void notifyNewDecision(Variable *x, Solver &s) { };   // Notify, when everything is done
};

class ObserverSingletonVariable {
   public:
    virtual void notifySingletonVariable(Variable *x) = 0;
};

class ObserverDeleteDecision {
   public:
    virtual void notifyDeleteDecision(Variable *x, int v, Solver &s) { };   // Notify, when everything is done
    virtual void notifyFullBacktrack() { }
};

}   // namespace Cosoco


#endif /* OBERSERNEWDECISION_H */
