//
// Created by audemard on 23/04/25.
//

#ifndef COSOCO_AMONG_H
#define COSOCO_AMONG_H
#include <set>

#include "constraints/globals/GlobalConstraint.h"

namespace Cosoco {
class Among : public GlobalConstraint {
    SparseSet mixedVariables;

   public:
    int           k;
    std::set<int> values;


    Among(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &values, int k);
    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool isCorrectlyDefined() override;
};
}   // namespace Cosoco


#endif   // COSOCO_AMONG_H
