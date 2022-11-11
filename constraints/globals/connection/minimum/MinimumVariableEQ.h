
#ifndef MINIMUMVARIABLEEQ_H
#define MINIMUMVARIABLEEQ_H

#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"

namespace Cosoco {

class MinimumVariableEQ : public GlobalConstraint {
   public:
    Variable       *value;
    vec<Variable *> list;

    MinimumVariableEQ(Problem &p, std::string n, vec<Variable *> &vars, Variable *x);


    bool filter(Variable *dummy) override;


    bool isSatisfiedBy(vec<int> &tuple) override;

   private:
    Variable       *findNewSentinelFor(int v, Variable *except);
    int             computeLimitForSentinel(Variable *sentinel);
    vec<Variable *> sentinels;   // sentinels for idxs of values of the extremum var
};
}   // namespace Cosoco

#endif /* MINIMUMVARIABLEEQ_H */
