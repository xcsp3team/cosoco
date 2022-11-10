//
// Created by audemard on 22/01/2021.
//

#ifndef COSOCO_SUMSCALARLEK_H
#define COSOCO_SUMSCALARLEK_H
#include "SumScalar.h"

namespace Cosoco {
class SumScalarLEK : public SumScalar {
   public:
    long limit;

    SumScalarLEK(Problem &p, std::string n, vec<Variable *> &variables, vec<Variable *> &coefs, long l);


    bool isCorrectlyDefined() override;


    bool filter(Variable *x) override;


    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco
#endif   // COSOCO_SUMSCALARLEK_H
