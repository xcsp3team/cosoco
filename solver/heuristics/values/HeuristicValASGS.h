//
// Created by audemard on 17/11/22.
//

#ifndef COSOCO_HEURISTICVALASGS_H
#define COSOCO_HEURISTICVALASGS_H

#include "HeuristicVal.h"

namespace Cosoco {
class HeuristicValASGS : public HeuristicVal {
   public:
    explicit HeuristicValASGS(Solver &s);

    int select(Variable *x) override;
};
}   // namespace Cosoco
#endif   // COSOCO_HEURISTICVALASGS_H
