#include "HeuristicVar.h"
#include "solver/Solver.h"
#include "solver/observers/ObserverConflict.h"
#include "solver/observers/ObserverDecision.h"

#ifndef COSOCO_HEURISTICVARFRBA_H
#define COSOCO_HEURISTICVARFRBA_H


namespace Cosoco {

struct dataFRBA {
    uint64_t nFailed;        // the number failed assignments per variable
    uint64_t nAssignments;   // the number  assignments per variable
    uint64_t lastFailed;     // the last failed assignement per variable
    double   operator()(uint64_t nFailedAssignments) {
        return (((double)nFailed) / (double)nAssignments) + (1 / (double)(nFailedAssignments - lastFailed + 1));
    }
};

class HeuristicVarFRBA : public HeuristicVar, ObserverConflict, ObserverNewDecision, ObserverDeleteDecision {
    vec<dataFRBA> data;   // the data for each variable
    uint64_t      nFailedAssignments;


   public:
    explicit HeuristicVarFRBA(Solver &s);
    Variable *select() override;
    void      notifyConflict(Constraint *c, int level) override;
    void      notifyDeleteDecision(Variable *x, int v, Solver &s) override;
    void      notifyFullBacktrack() override;
    void      notifyNewDecision(Variable *x, Solver &s) override;
};

}   // namespace Cosoco


#endif   // COSOCO_HEURISTICVARFRBA_H
