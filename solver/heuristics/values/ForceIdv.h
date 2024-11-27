#ifndef COSOCO_FORCEVALUE_H
#define COSOCO_FORCEVALUE_H
#include <solver/observers/ObserverDecision.h>

#include "HeuristicVal.h"


namespace Cosoco {
// Try to assign value
class ForceIdvs : public HeuristicVal, ObserverDeleteDecision {
   protected:
    vec<int>      idvs;
    HeuristicVal *hv;
    bool          conflictAlreadySeen;
    bool          onlyOnce;
    bool          isDisabled          = false;
    uint64_t      restartWithSolution = -1;
    int           rr                  = 0;

   public:
    ForceIdvs(Solver &s, HeuristicVal *h, bool oo = true, vec<int> *values = nullptr);
    int select(Variable *x) override;

    void notifyFullBacktrack() override;
    void setIdValues(vec<int> &v);
};
}   // namespace Cosoco


#endif   // COSOCO_FORCEVALUE_H
