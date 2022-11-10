
#ifndef MAXIMUMVARIABLEEQ_H
#define MAXIMUMVARIABLEEQ_H

#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"

namespace Cosoco {

class MaximumVariableEQ : public GlobalConstraint {
   public:
    Variable *      value;
    vec<Variable *> list;

    MaximumVariableEQ(Problem &p, std::string n, vec<Variable *> &vars, Variable *x);


    virtual bool filter(Variable *dummy) override;


    virtual bool isSatisfiedBy(vec<int> &tuple) override;

   private:
    Variable *      findNewSentinelFor(int v, Variable *except);
    int             computeLimitForSentinel(Variable *sentinel);
    vec<Variable *> sentinels;   // sentinels for idxs of values of the extremum var
};
}   // namespace Cosoco

#endif /* MAXIMIZECONSTANT_H */
