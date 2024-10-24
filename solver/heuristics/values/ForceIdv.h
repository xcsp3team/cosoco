#ifndef COSOCO_FORCEVALUE_H
#define COSOCO_FORCEVALUE_H
#include <solver/observers/ObserverConflict.h>

#include "HeuristicVal.h"
#include "ObserverDecision.h"


namespace Cosoco {
// Try to assign value
class ForceIdvs : public HeuristicVal, ObserverDeleteDecision {
   protected:
    vec<int>      idvs;
    HeuristicVal *hv;

    bool     isDisabled          = false;
    uint64_t restartWithSolution = -1;
    bool     rr                  = false;

   public:
    ForceIdvs(Solver &s, HeuristicVal *h, bool oo = true, vec<int> *values = nullptr);
    int select(Variable *x) override;

    void setIdValues(vec<int> &v);
    void notifyFullBacktrack() override;
};
}   // namespace Cosoco


#endif   // COSOCO_FORCEVALUE_H
