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
    virtual void updateBound(long bound) override;

    virtual long maxUpperBound() override;

    virtual long minLowerBound() override;

    virtual long computeScore(vec<int> &tuple) override;
};
}   // namespace Cosoco

#endif /* MAXIMIZECONSTANT_H */
