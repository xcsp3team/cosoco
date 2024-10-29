#ifndef MINIMIZECONSTANT_H
#define MINIMIZECONSTANT_H

#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"
#include "optimizer/ObjectiveConstraint.h"
namespace Cosoco {

class MinimumConstant : public GlobalConstraint, public ObjectiveConstraint {
   public:
    int k;

    MinimumConstant(Problem &p, std::string n, vec<Variable *> &vars, int kk)
        : GlobalConstraint(p, n, "Min constant", vars), k(kk) { }

    // Function related to optimisation (see ObjectiveConstraint class)
    void updateBound(long bound) override;
    long maxUpperBound() override;
    long minLowerBound() override;
    long computeScore(vec<int> &tuple) override;
};
}   // namespace Cosoco

#endif /* MINIMIZECONSTANT_H */
