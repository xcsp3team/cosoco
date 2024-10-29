#ifndef COSOCO_HEURISTICVALSTICKINGVALUE_H
#define COSOCO_HEURISTICVALSTICKINGVALUE_H
#include <memory>

#include "HeuristicVal.h"


namespace Cosoco {
class HeuristicValStickingValue : public HeuristicVal {
   protected:
    vec<int>                      lastValue;
    std::unique_ptr<HeuristicVal> hv;

   public:
    HeuristicValStickingValue(Solver &s, std::unique_ptr<HeuristicVal> &&h);
    virtual int select(Variable *x) override;
};
}   // namespace Cosoco


#endif   // COSOCO_HEURISTICVALSTICKINGVALUE_H
