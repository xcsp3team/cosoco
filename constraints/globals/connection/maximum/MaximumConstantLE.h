#ifndef MAXIMUMCONSTANTLE_H
#define MAXIMUMCONSTANTLE_H

#include "MaximumConstant.h"
#include "mtl/Vec.h"
namespace Cosoco {
class MaximumConstantLE : public MaximumConstant {
   public:
    MaximumConstantLE(Problem &p, vec<Variable *> &vars, int kk) : MaximumConstant(p, vars, kk) { type = "Maximum Constant LE"; }

    bool filter(Variable *dummy) override;
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif /* MAXIMUMCONSTANTLE_H */
