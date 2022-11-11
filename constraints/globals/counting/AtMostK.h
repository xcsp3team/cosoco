#ifndef ATMOSTK_H
#define ATMOSTK_H
#include "constraints/globals/GlobalConstraint.h"


namespace Cosoco {

class AtMostK : public GlobalConstraint {
   public:
    int k;
    int value;

    AtMostK(Problem &p, std::string n, vec<Variable *> &vars, int k, int val);
    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool isCorrectlyDefined() override;
};
}   // namespace Cosoco


#endif /* ATMOSTK_H */
