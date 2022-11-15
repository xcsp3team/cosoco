#ifndef COSOCO_CIRCUIT_H
#define COSOCO_CIRCUIT_H

#include <constraints/globals/comparison/AllDifferent.h>

namespace Cosoco {
class Circuit : public AllDifferent {
   public:
    SparseSet set;
    vec<bool> tmp;   // Temporary, avoid to create it at each filter calls

    Circuit(Problem &p, std::string n, vec<Variable *> &vars);

    bool isCorrectlyDefined() override;
    bool filter(Variable *dummy) override;
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif   // COSOCO_CIRCUIT_H
