//
// Created by audemard on 10/12/2020.
//

#ifndef COSOCO_HEURISTICVARCACD_H
#define COSOCO_HEURISTICVARCACD_H


#include <solver/observers/ObserverConflict.h>
#include <solver/observers/ObserverDecision.h>

#include "HeuristicVar.h"

namespace Cosoco {

class HeuristicVarCACD : public HeuristicVar, ObserverConflict, ObserverNewDecision, ObserverDeleteDecision {
    vec<double> vscores;   // the score of all variables
    vec<double> cscores;   // the score of constraints (mainly used for CHS)

    vec<vec<double> > cvscores;

   public:
    explicit HeuristicVarCACD(Solver &s);
    void      reset();
    Variable *select() override;
    void      notifyConflict(Constraint *c, int level) override;
    void      notifyDeleteDecision(Variable *x, int v, Solver &s) override;
    void      notifyFullBacktrack() override;
    void      notifyNewDecision(Variable *x, Solver &s) override;
};

}   // namespace Cosoco

#endif   // COSOCO_HEURISTICVARCACD_H
