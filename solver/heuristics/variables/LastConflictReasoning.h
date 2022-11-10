#ifndef HEURISTICLASTCONFLICTREASONING_H
#define HEURISTICLASTCONFLICTREASONING_H


#include "HeuristicVar.h"
#include "solver/Solver.h"
#include "solver/observers/ObserverConflict.h"
namespace Cosoco {
class LastConflictReasoning : public HeuristicVar, ObserverConflict {
   public:
    HeuristicVar *hvar;   // The main heuristic
    vec<Variable *>    lcs;     // The last conflict variable
    LastConflictReasoning(Solver &s, HeuristicVar *hv, int _nbVariables = 2);
    int nVariables;

    virtual Variable *select() override;
    virtual void      notifyConflict(Constraint *c, int level) override;
};

}   // namespace Cosoco

#endif /* HEURISTICVARDOMWDEG_H */
