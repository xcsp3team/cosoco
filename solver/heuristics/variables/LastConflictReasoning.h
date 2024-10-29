#ifndef HEURISTICLASTCONFLICTREASONING_H
#define HEURISTICLASTCONFLICTREASONING_H


#include "HeuristicVar.h"
#include "solver/Solver.h"
#include "solver/observers/ObserverConflict.h"
namespace Cosoco {
class LastConflictReasoning : public HeuristicVar, ObserverNewDecision, ObserverDeleteDecision {
    std::unique_ptr<HeuristicVar> hvar;   // The main heuristic
    vec<Variable *>               lcs;    // The last conflict variable
    int                           nVariables;
    Variable                     *lastAssigned, *candidate;

   public:
    LastConflictReasoning(Solver &s, std::unique_ptr<HeuristicVar> &&hv, int _nbVariables = 2);

    Variable *select() override;

    void notifyNewDecision(Variable *x, Solver &s) override;
    void notifyDeleteDecision(Variable *x, int v, Solver &s) override;
};

}   // namespace Cosoco

#endif /* HEURISTICVARDOMWDEG_H */
