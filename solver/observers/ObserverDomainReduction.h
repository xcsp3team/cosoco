#ifndef COSOCO_OBSERVERDOMAINREDUCTION_H
#define COSOCO_OBSERVERDOMAINREDUCTION_H


namespace Cosoco {
class Solver;
class Variable;
class ObserverDomainReduction {
   public:
    virtual ~ObserverDomainReduction()                                   = default;
    virtual void notifyDomainReduction(Variable *x, int idv, Solver &s)  = 0;   // event is sent before the value is removed
    virtual void notifyDomainAssignment(Variable *x, int idv, Solver &s) = 0;   // event is sent before the value is assigned
};
}   // namespace Cosoco

#endif   // COSOCO_OBSERVERDOMAINREDUCTION_H
