#ifndef COSOCO_XADDYEQZ_H
#define COSOCO_XADDYEQZ_H


#include "EQ.h"
#include "constraints/Ternary.h"

namespace Cosoco {
class xAddyEQz : public Ternary {
    EQ *eq;

   public:
    // Constructors
    xAddyEQz(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz);

    // filtering
    bool filter(Variable *x) override;
    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;

    void attachSolver(Solver *s) override;
};
}   // namespace Cosoco


#endif
