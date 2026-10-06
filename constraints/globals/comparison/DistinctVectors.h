#ifndef COSOCO_DISTINCTVECTORS_H
#define COSOCO_DISTINCTVECTORS_H


#include "ObserverDecision.h"
#include "SparseSetMultiLevel.h"
#include "constraints/globals/GlobalConstraint.h"


namespace Cosoco {
class DistinctVectors : public GlobalConstraint {
   protected:
    int                          sentinel1, sentinel2;   // Two sentinels for tracking the presence of different values.
    vec<Variable *>              X, Y;                   // X != Y
    int                          size;                   // The size of vectors
    VariablePositionInConstraint variablePosition;


    bool isSentinel(int i);

    int findAnotherSentinel();

    bool isPossibleInferenceFor(int sentinel);

    void handlePossibleInferenceFor(int sentinel);

   public:
    DistinctVectors(Problem &p, vec<Variable *> &XX, vec<Variable *> &YY);
    void delayedConstruction(int id) override;

    bool isCorrectlyDefined() override;

    bool isSatisfiedBy(vec<int> &tuple) override;

    bool filter(Variable *x) override;
};

class DistinctVectorsK : public GlobalConstraint, ObserverDeleteDecision {
   protected:
    vec<vec<Variable *>>         lists;
    int                          n, m;
    int                         *rows, *cols;
    VariablePositionInConstraint variablePosition;

    int  *offsets;
    int **sentinels;

    // Notifications : restore validTuples when backtrack is performed
    void notifyDeleteDecision(Variable *x, int v, Solver &s, bool isFull) override;


    SparseSetMultiLevel set;

    int findSentinel(int i, int ii, int jToIgnore);
    int isSentinelFor(int i, int ii, int j);

   public:
    DistinctVectorsK(Problem &p, vec<vec<Variable *>> &XX);
    void delayedConstruction(int id) override;
    bool isCorrectlyDefined() override;
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool filter(Variable *x) override;
};

}   // namespace Cosoco


#endif   // COSOCO_DISTINCTVECTORS_H
