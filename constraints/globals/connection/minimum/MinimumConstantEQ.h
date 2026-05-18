//
// Created by audemard on 24/11/2025.
//

#ifndef COSOCO_MINIMUMCONSTANTEQ_H
#define COSOCO_MINIMUMCONSTANTEQ_H
#include "MinimumConstant.h"

namespace Cosoco {
class MinimumConstantEQ : public MinimumConstant {
    Variable *sentinel1, *sentinel2;

   public:
    MinimumConstantEQ(Problem &p, std::string n, vec<Variable *> &vars, int kk) : MinimumConstant(p, n, vars, kk) {
        sentinel1 = scope[0];
        sentinel2 = scope[1];
        type      = "Minimum Constant EQ";
    }


    bool      filter(Variable *dummy) override;
    bool      isSatisfiedBy(vec<int> &tuple) override;
    Variable *findNewSentinel();
};
}   // namespace Cosoco

#endif   // COSOCO_MINIMUMCONSTANTEQ_H
