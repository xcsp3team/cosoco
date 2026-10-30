#ifndef SUMEQ_H
#define SUMEQ_H

#include "Sum.h"
namespace Cosoco {

class SumWeightedEQ : public WeightedSum {
   public:
    SumWeightedEQ(Problem &p, vec<Variable *> &vars, vec<int> &coefs, long l);
    bool filter(Variable *x) override;
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};


class SumSimpleEQ : public SimpleSum {
   public:
    SumSimpleEQ(Problem &p, vec<Variable *> &vars, long l);
    bool filter(Variable *x) override;
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco

#endif /* SUMEQ_H */
