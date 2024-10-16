#ifndef COSOCO_DIV_H
#define COSOCO_DIV_H

#include "constraints/Binary.h"

namespace Cosoco {
class DivLE : public Binary {
   public:
    int k;
    // Constructors
    DivLE(Problem &p, std::string n, Variable *xx, int kk, Variable *yy);

    // filtering
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
class DivGE : public Binary {
   public:
    int k;
    // Constructors
    DivGE(Problem &p, std::string n, Variable *xx, int kk, Variable *yy);

    // filtering
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

}   // namespace Cosoco
#endif   // COSOCO_DIV_H
