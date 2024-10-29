#ifndef COSOCO_FORCEVALUE_H
#define COSOCO_FORCEVALUE_H
#include <solver/observers/ObserverConflict.h>

#include <memory>

#include "HeuristicVal.h"


namespace Cosoco {
// Try to assign value
class ForceIdvs : public HeuristicVal, ObserverConflict {
   protected:
    vec<int>                      idvs;
    std::unique_ptr<HeuristicVal> hv;
    bool                          conflictAlreadySeen;
    bool                          onlyOnce;

   public:
    ForceIdvs(Solver &s, std::unique_ptr<HeuristicVal> &&h, bool oo = true, vec<int> *values = nullptr);
    int select(Variable *x) override;

    void notifyConflict(Constraint *c, int level) override;
    void setIdValues(vec<int> &v);
};
}   // namespace Cosoco


#endif   // COSOCO_FORCEVALUE_H
