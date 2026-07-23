#include "Constraint.h"
#include "core/Variable.h"
#ifndef BINARY_H
#define BINARY_H

namespace Cosoco {
class Binary : public Constraint {
   public:
    Variable *x, *y;

    Binary(Problem &p, Variable *xx, Variable *yy) : Constraint(p, createScopeVec(xx, yy)), x(xx), y(yy) { }
};

}   // namespace Cosoco


#endif /* BINARY_H */
