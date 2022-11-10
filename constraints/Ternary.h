#ifndef TERNARY_H
#define TERNARY_H
#include "Constraint.h"
#include "core/Variable.h"
namespace Cosoco {
class Ternary : public Constraint {
   public:
    Variable *x, *y, *z;

    Ternary(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz)
        : Constraint(p, n, xx, yy, zz), x(xx), y(yy), z(zz) { }
};

}   // namespace Cosoco

#endif /* TERNARY_H */
