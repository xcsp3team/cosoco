#ifndef MAXIMIZECONSTANT_H
#define MAXIMIZECONSTANT_H

#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"
#include "optimizer/ObjectiveConstraint.h"

namespace Cosoco {

class MaximumConstant : public GlobalConstraint, public ObjectiveConstraint {
   protected:
   public:
    int k;


    MaximumConstant(Problem &p, std::string n, vec<Variable *> &vars, int kk)
        : GlobalConstraint(p, n, "Max constant", vars), k(kk) { }


    // Function related to optimisation (see ObjectiveConstraint class)
    void updateBound(long bound) override;

    long maxUpperBound() override;

    long minLowerBound() override;

    long computeScore(vec<int> &tuple) override;
};
}   // namespace Cosoco

#endif /* MAXIMIZECONSTANT_H */
