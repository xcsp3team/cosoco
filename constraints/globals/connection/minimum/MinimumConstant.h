#ifndef MINIMIZECONSTANT_H
#define MINIMIZECONSTANT_H

#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"
namespace Cosoco {

class MinimumConstant : public GlobalConstraint, public ObjectiveConstraint {
   public:
    int k;

    MinimumConstant(Problem &p, std::string n, vec<Variable *> &vars, int kk)
        : GlobalConstraint(p, n, "Min constant", vars), k(kk) { }

    // Function related to optimisation (see ObjectiveConstraint class)
    virtual void updateBound(long bound) override;
    virtual long maxUpperBound() override;
    virtual long minLowerBound() override;
    virtual long computeScore(vec<int> &tuple) override;
};
}   // namespace Cosoco

#endif /* MAXIMIZECONSTANT_H */
