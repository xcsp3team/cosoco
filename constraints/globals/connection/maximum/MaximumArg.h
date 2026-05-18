//
// Created by audemard on 19/11/2025.
//

#ifndef COSOCO_MAXIMUMARG_H
#define COSOCO_MAXIMUMARG_H
#include <XCSP3Constants.h>

#include "GlobalConstraint.h"

namespace Cosoco {
class MaximumArg : public GlobalConstraint {
   public:
    Variable           *index;
    vec<Variable *>     list;
    XCSP3Core::RankType rank;
    MaximumArg(Problem &p, std::string n, vec<Variable *> &vars, Variable *idx, XCSP3Core::RankType r);
    bool filter(Variable *dummy) override;
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco
#endif   // COSOCO_MAXIMUMARG_H
