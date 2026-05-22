#ifndef HEURISTICVARDOMWDEG_H
#define HEURISTICVARDOMWDEG_H


#include "HeuristicVar.h"
#include "solver/Solver.h"
#include "solver/observers/ObserverConflict.h"
#include "solver/observers/ObserverDecision.h"

enum Mode { V2004, ABSCON, NEWWDEG, V2021 };

namespace Cosoco {
class HeuristicVarDomWdeg : public HeuristicVar, ObserverConflict, ObserverNewDecision, ObserverDeleteDecision {
    vec<vec<double>> constraintsWeights;
    vec<double>      variablesWeights;

   public:
    explicit HeuristicVarDomWdeg(Solver &s);
    Mode      mode;
    Variable *select() override;
    void      notifyConflict(Constraint *c, int level) override;
    void      notifyDeleteDecision(Variable *x, int v, Solver &s, bool isFull) override;
    void      notifyFullBacktrack() override;
    void      notifyNewDecision(Variable *x, Solver &s) override;
    bool      stop() override;
    bool      start() override;
};

}   // namespace Cosoco

#endif /* HEURISTICVARDOMWDEG_H */
