#ifndef COSOCO_HEURISTICVALLAST_H
#define COSOCO_HEURISTICVALLAST_H


#include "HeuristicVal.h"

namespace Cosoco {
class HeuristicValLast : public HeuristicVal {
   public:
    HeuristicValLast(Solver &s);
    virtual int select(Variable *x) override;
};
}   // namespace Cosoco
#endif   // COSOCO_HEURISTICVALLAST_H
