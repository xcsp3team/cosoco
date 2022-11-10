#ifndef COSOCO_DISTINCTVECTORS_H
#define COSOCO_DISTINCTVECTORS_H


#include "constraints/globals/GlobalConstraint.h"


namespace Cosoco {
class DistinctVectors : public GlobalConstraint {
   protected:
    int             sentinel1, sentinel2;   // Two sentinels for tracking the presence of different values.
    vec<Variable *> X, Y;                   // X != Y
    int             size;                   // The size of vectors

    bool isSentinel(int i);

    int findAnotherSentinel();

    bool isPossibleInferenceFor(int sentinel);

    void handlePossibleInferenceFor(int sentinel);

   public:
    DistinctVectors(Problem &p, std::string n, vec<Variable *> &XX, vec<Variable *> &YY);

    bool isCorrectlyDefined() override;

    bool isSatisfiedBy(vec<int> &tuple) override;

    bool filter(Variable *x) override;
};
}   // namespace Cosoco


#endif   // COSOCO_DISTINCTVECTORS_H
