#ifndef MAXIMIZEVARIABLE_H
#define MAXIMIZEVARIABLE_H

#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"

namespace Cosoco {

class MaximumVariable : public GlobalConstraint {
   public:
    Variable *maxVar;


    MaximumVariable(Problem &p, std::string n, vec<Variable *> &vars, Variable *v)
        : GlobalConstraint(p, n, "Max variable", Constraint::createScopeVec(&vars, v)), maxVar(v) { }
};
}   // namespace Cosoco


#endif /* MAXIMIZEVARIABLE_H */
