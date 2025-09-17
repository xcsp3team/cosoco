//
// Created by audemard on 14/02/23.
//

#ifndef COSOCO_PICKONDOM_H
#define COSOCO_PICKONDOM_H
#include "HeuristicVar.h"
#include "solver/Solver.h"
#include "solver/observers/ObserverConflict.h"
#include "solver/observers/ObserverDecision.h"

namespace Cosoco {
class PickOnDom : public HeuristicVar, ObserverConflict, ObserverDeleteDecision {
    unsigned long next;
    int           mode = 0;
    vec<double>   variablesWeights;

   public:
    PickOnDom(Solver &s);
    Variable *select() override;
    void      notifyConflict(Constraint *c, int level) override;
    void      notifyFullBacktrack() override;
    bool      stop() override;
    bool      start() override;
};

}   // namespace Cosoco

#endif   // COSOCO_PICKONDOM_H
