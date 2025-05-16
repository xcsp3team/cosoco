#ifndef COSOCO_HEURISTICVALSTICKINGVALUE_H
#define COSOCO_HEURISTICVALSTICKINGVALUE_H
#include "HeuristicVal.h"
#include "ObserverDecision.h"


namespace Cosoco {
class HeuristicValStickingValue : public HeuristicVal, ObserverSingletonVariable {
   protected:
    vec<int>      lastValue;
    HeuristicVal *hv;

   public:
    HeuristicValStickingValue(Solver &s, HeuristicVal *h);
    int  select(Variable *x) override;
    void notifySingletonVariable(Variable *x) override;
};
}   // namespace Cosoco


#endif   // COSOCO_HEURISTICVALSTICKINGVALUE_H
