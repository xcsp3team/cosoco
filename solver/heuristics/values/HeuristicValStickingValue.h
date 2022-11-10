#ifndef COSOCO_HEURISTICVALSTICKINGVALUE_H
#define COSOCO_HEURISTICVALSTICKINGVALUE_H
#include "HeuristicVal.h"


namespace Cosoco {
class HeuristicValStickingValue : public HeuristicVal {
   protected:
    vec<int>      lastValue;
    HeuristicVal *hv;

   public:
    HeuristicValStickingValue(Solver &s, HeuristicVal *h);
    virtual int select(Variable *x) override;
};
}   // namespace Cosoco


#endif   // COSOCO_HEURISTICVALSTICKINGVALUE_H
