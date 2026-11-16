#ifndef DIST2_H
#define DIST2_H


#include "constraints/Binary.h"

namespace Cosoco {

class DistNE : public Binary {
   public:
    int k;
    // Constructors
    DistNE(Problem &p, Variable *xx, Variable *yy, int kk);

    // filtering
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;

    //
    bool revise(Variable *z1, Variable *z2);
    bool isASupportFor(int vy, Variable *z);
};
class DistEQ : public Binary {
   public:
    int k;
    // Constructors
    DistEQ(Problem &p, Variable *xx, Variable *yy, int kk);

    // filtering
    bool filter(Variable *x) override;
    bool removeAtDistanceNE(Variable *x, Variable *y);
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif /* DISTNE_H */
