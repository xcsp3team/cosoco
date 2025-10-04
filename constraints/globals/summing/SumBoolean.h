//
// Created by audemard on 04/10/2025.
//

#ifndef COSOCO_SUMBOOLEAN_H
#define COSOCO_SUMBOOLEAN_H


#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"
namespace Cosoco {

class SumBoolean : public GlobalConstraint {
   public:
    long limit;

    SumBoolean(Problem &p, std::string n, vec<Variable *> &vars, long l);
    bool isCorrectlyDefined() override;
};


class SumBooleanEQ : public SumBoolean {
   public:
    SumBooleanEQ(Problem &p, std::string n, vec<Variable *> &vars, long l);
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool filter(Variable *x) override;
};

class SumBooleanLE : public SumBoolean {
   public:
    SumBooleanLE(Problem &p, std::string n, vec<Variable *> &vars, long l);
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool filter(Variable *x) override;
};
}   // namespace Cosoco


#endif   // COSOCO_SUMBOOLEAN_H
