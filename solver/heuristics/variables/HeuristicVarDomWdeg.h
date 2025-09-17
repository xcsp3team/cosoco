#ifndef HEURISTICVARDOMWDEG_H
#define HEURISTICVARDOMWDEG_H


#include "HeuristicVar.h"
#include "solver/Solver.h"
#include "solver/observers/ObserverConflict.h"
#include "solver/observers/ObserverDecision.h"

enum Mode { V2004, ABSCON, NEWWDEG, V2021 };

namespace Cosoco {
class HeuristicVarDomWdeg : public HeuristicVar, ObserverConflict, ObserverNewDecision, ObserverDeleteDecision {
    unsigned long    next;
    vec<vec<double>> constraintsWeights;
    vec<double>      variablesWeights;

   public:
    explicit HeuristicVarDomWdeg(Solver &s);
    Mode              mode;
    virtual Variable *select() override;
    virtual void      notifyConflict(Constraint *c, int level) override;
    virtual void      notifyDeleteDecision(Variable *x, int v, Solver &s) override;
    void              notifyFullBacktrack() override;
    virtual void      notifyNewDecision(Variable *x, Solver &s) override;
    bool              stop() override;
    bool              start() override;
};

}   // namespace Cosoco

#endif /* HEURISTICVARDOMWDEG_H */
