#ifndef MINIMUMCONSTANTLE_H
#define MINIMUMCONSTANTLE_H

#include "MinimumConstant.h"
#include "mtl/Vec.h"
namespace Cosoco {
class MinimumConstantLE : public MinimumConstant {
   protected:
    int sentinel1, sentinel2;

   public:
    MinimumConstantLE(Problem &p, std::string n, vec<Variable *> &vars, int kk) : MinimumConstant(p, n, vars, kk) {
        sentinel1 = 0;
        sentinel2 = vars.size() - 1;
    }

    bool filter(Variable *dummy) override;
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif /* MAXIMUMCONSTANTLE_H */
