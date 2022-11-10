#ifndef MAXIMIZEVARIABLE_H
#define MAXIMIZEVARIABLE_H

#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"

namespace Cosoco {

class MaximumVariable : public GlobalConstraint {
   public:
    Variable *maxVar;


    MaximumVariable(Problem &p, std::string n, vec<Variable *> &vars, Variable *v)
        : GlobalConstraint(p, n, "Max variable", vars.size() + 1), maxVar(v) {
        vars.push(maxVar);
        scopeInitialisation(vars);
        vars.pop();   // leave vars unchanged after call
    }
};
}   // namespace Cosoco


#endif /* MAXIMIZEVARIABLE_H */
