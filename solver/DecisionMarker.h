
#ifndef COSOCO_DECISIONMARKER_H
#define COSOCO_DECISIONMARKER_H


// This class is usef to store the current branch of the search with negative decisions
// Usefull for nogoods recording....

#include <mtl/Vec.h>

#include "ObserverDecision.h"
#include "Solver.h"

namespace Cosoco {
class DecisionMarker : public ObserverNewDecision, ObserverDeleteDecision {
   protected:
    int       OFFSET;
    vec<long> currentBranch;   // each long represents x=v or x!=v (given the offset).
    Solver *  solver;


   public:
    DecisionMarker(Solver *s);

    virtual void notifyNewDecision(Variable *x, Solver &s) override;
    virtual void notifyDeleteDecision(Variable *x, int v, Solver &s) override;
    virtual void notifyFullBacktrack() override;

    int getPositiveDecisionFor(Variable *x, int idv);
    int getNegativeDecisionFor(Variable *x, int idv);

    Variable *getVariableIn(int number);
    int       getIndexIn(int number);

    int depth();

    bool generateNogoodsFromRestarts();

    std::string getStringFor(int dec);
    void        display();
};
}   // namespace Cosoco


#endif   // COSOCO_DECISIONMARKER_H
