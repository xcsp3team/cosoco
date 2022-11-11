#ifndef ATLEASTK_H
#define ATLEASTK_H
#include "constraints/globals/GlobalConstraint.h"

namespace Cosoco {

class AtLeastK : public GlobalConstraint {
   public:
    int       k;
    int       value;
    SparseSet sentinels;   // The sentinels saying t


    AtLeastK(Problem &p, std::string n, vec<Variable *> &vars, int k, int val);
    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool isCorrectlyDefined() override;
};
}   // namespace Cosoco


#endif /* ATLEASTK_H */
