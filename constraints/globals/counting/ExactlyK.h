//
// Created by audemard on 10/12/2015.
//

#ifndef COSOCO_EXACTLYK_H
#define COSOCO_EXACTLYK_H

#include <constraints/globals/GlobalConstraint.h>

namespace Cosoco {

class ExactlyK : public GlobalConstraint {
    int  k;
    int  value;
    bool done;
    vec<int>valueToidv;
   public:
    ExactlyK(Problem &p, std::string n, vec<Variable *> &vars, int k, int val);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;

    bool isCorrectlyDefined() override;

    State status() override;

    void reinitialize() override;
};
}   // namespace Cosoco


#endif   // COSOCO_EXACTLYK_H
