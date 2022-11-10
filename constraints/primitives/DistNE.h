#ifndef DISTNE_H
#define DISTNE_H


#include "constraints/Binary.h"

namespace Cosoco {

class DistNE : public Binary {
   public:
    int k;
    // Constructors
    DistNE(Problem &p, std::string n, Variable *xx, Variable *yy, int kk);

    // filtering
    bool filter(Variable *x) override;

    // Checking
    virtual bool isSatisfiedBy(vec<int> &tuple) override;

    //
    bool revise(Variable *z1, Variable *z2);
    bool isASupportFor(int vy, Variable *z);
};
}   // namespace Cosoco


#endif /* DISTNE_H */
