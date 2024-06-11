//
// Created by audemard on 11/06/24.
//

#ifndef COSOCO_HEURISTICVARCRBS_H
#define COSOCO_HEURISTICVARCRBS_H

#include "HeuristicVar.h"
#include "Matrix.h"
#include "ObserverConflict.h"
#include "ObserverDecision.h"
namespace Cosoco {
class HeuristicVarCRBS : public HeuristicVar, ObserverConflict, ObserverNewDecision, ObserverDeleteDecision {
    double       theta;
    Matrix<int> *a;   // correlation matrix

   public:
    HeuristicVarCRBS(Solver &s);
    Variable *select() override;
    double    score(Variable *x);
    void      notifyConflict(Constraint *c, int level) override;
    void      notifyFullBacktrack() override;
    void      notifyNewDecision(Variable *x, Solver &s) override;
};
}   // namespace Cosoco


#endif   // COSOCO_HEURISTICVARCRBS_H
