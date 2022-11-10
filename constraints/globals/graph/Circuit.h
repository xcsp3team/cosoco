#ifndef COSOCO_CIRCUIT_H
#define COSOCO_CIRCUIT_H

#include <constraints/globals/comparison/AllDifferent.h>

namespace Cosoco {
class Circuit : public AllDifferent {
   public:
    SparseSet set;

    Circuit(Problem &p, std::string n, vec<Variable *> &vars);
    virtual bool filter(Variable *dummy) override;
    virtual bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif   // COSOCO_CIRCUIT_H