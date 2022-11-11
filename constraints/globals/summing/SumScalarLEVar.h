//
// Created by audemard on 22/01/2021.
//

#ifndef COSOCO_SUMSCALARLEVar_H
#define COSOCO_SUMSCALARLEVar_H
#include "SumScalar.h"

namespace Cosoco {
class SumScalarLEVar : public SumScalar {
   public:
    Variable *limit;
    SumScalarLEVar(Problem &p, std::string n, vec<Variable *> &variables, vec<Variable *> &coefs, Variable *z);

    bool isCorrectlyDefined() override;

    bool filter(Variable *x) override;


    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco
#endif   // COSOCO_SUMSCALARLEVar_H
