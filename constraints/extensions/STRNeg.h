#ifndef COSOCO_STRNEG_H
#define COSOCO_STRNEG_H

#include "Extension.h"
#include "mtl/SparseSet.h"
#include "mtl/SparseSetMultiLevel.h"
#include "solver/observers/ObserverDecision.h"


namespace Cosoco {
class STRNeg : public Extension, ObserverDeleteDecision {
   protected:
    SparseSetMultiLevel validTuples;   // Valid tupes aka valid wrt interpretation not related to conflict tuples

    vec<long>      nbValidTuples;    // 1D = variable position
    vec<vec<int> > nbConflicts;      // 1D = variable position ; 2D = index
    vec<int>       maxNbConflicts;   // 1D = variable position
    SparseSet      variablesToCheck;

   public:
    // Constructors and construction
    STRNeg(Problem &p, std::string n, vec<Variable *> &vars);
    STRNeg(Problem &p, std::string n, vec<Variable *> &vars, vec<vec<int> > &tuplesFromOtherConstraint);
    void delayedConstruction(int id) override;
    bool isCorrectlyDefined() override;
    void attachSolver(Solver *s) override;

    // filtering
    void initializeStructuresBeforeFiltering();
    bool filter(Variable *x) override;

    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool isValidTuple(vec<int> &tuple);


    // Notifications : restore validTuples when backtrack is performed
    void notifyDeleteDecision(Variable *x, int v, Solver &s) override;
    void delTuple(int position, int level);
};
}   // namespace Cosoco


#endif   // COSOCO_STRNEG_H
