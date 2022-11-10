#ifndef GLOBALCONSTRAINT_H
#define GLOBALCONSTRAINT_H

#include "constraints/Constraint.h"
namespace Cosoco {

class GlobalConstraint : public Constraint {
   public:
    GlobalConstraint(Problem &p, std::string n, std::string t, vec<Variable *> &vars) : Constraint(p, n, vars) { type = t; }
    GlobalConstraint(Problem &p, std::string n, std::string t, int sz) : Constraint(p, n, sz) {
        type = t;
    }   // Scope initialisation is postponed
};
}   // namespace Cosoco

#endif /* GLOBALCONSTRAINT_H */
