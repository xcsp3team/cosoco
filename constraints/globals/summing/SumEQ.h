#ifndef SUMEQ_H
#define SUMEQ_H

#include "Sum.h"
namespace Cosoco {

class SumEQ : public Sum {
   public:
    SumEQ(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &coefs, long l);
    bool filter(Variable *x) override;


    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco

#endif /* SUMEQ_H */
