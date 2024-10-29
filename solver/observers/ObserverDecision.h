#ifndef OBERSERDECISION_H
#define OBERSERDECISION_H
#include "core/Variable.h"
namespace Cosoco {
class Solver;

class ObserverNewDecision {
   public:
    virtual ~ObserverNewDecision() = default;
    virtual void notifyNewDecision(Variable *x, Solver &s) {};   // Notify, when everything is done
};

class ObserverDeleteDecision {
   public:
    virtual ~ObserverDeleteDecision() = default;
    virtual void notifyDeleteDecision(Variable *x, int v, Solver &s) {};   // Notify, when everything is done
    virtual void notifyFullBacktrack() { }
};

}   // namespace Cosoco


#endif /* OBERSERNEWDECISION_H */
