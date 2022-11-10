#include "Constraint.h"
#include "core/Variable.h"
#ifndef BINARY_H
#define BINARY_H

namespace Cosoco {
class Binary : public Constraint {
   public:
    Variable *x, *y;

    Binary(Problem &p, std::string n, Variable *xx, Variable *yy) : Constraint(p, n, xx, yy), x(xx), y(yy) { }
};

}   // namespace Cosoco


#endif /* BINARY_H */
