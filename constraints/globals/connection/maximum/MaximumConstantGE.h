#ifndef MAXIMUMCONSTANTGE_H
#define MAXIMUMCONSTANTGE_H

#include "MaximumConstant.h"
#include "mtl/Vec.h"

namespace Cosoco {
class MaximumConstantGE : public MaximumConstant {
   protected:
    int sentinel1, sentinel2;

   public:
    MaximumConstantGE(Problem &p, std::string n, vec<Variable *> &vars, int kk) : MaximumConstant(p, n, vars, kk) {
        sentinel1 = 0;
        sentinel2 = vars.size() - 1;
        type = "Maximum Constant GE";
    }


    bool filter(Variable *dummy) override;

    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif /* MAXIMUMCONSTANTGE_H */
