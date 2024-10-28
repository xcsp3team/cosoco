#ifndef INTENSION_H
#define INTENSION_H

#ifdef USE_XCSP3
#include <map>

#include "constraints/Constraint.h"
#include "optimizer/ObjectiveConstraint.h"


namespace XCSP3Core {
class Tree;
}


namespace Cosoco {
class Intension : public Constraint, public ObjectiveConstraint {
   protected:
    XCSP3Core::Tree           *evaluator;
    std::map<std::string, int> tuple;

   public:
    // Constructors
    Intension(Problem &p, std::string n, XCSP3Core::Tree *tree, vec<Variable *> &scope)
        : Constraint(p, n, scope), evaluator(tree) {
        type = "Intension";
    }


    // filtering
    bool filter(Variable *x) override;

    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;


    // Objective functions
    // The constraint is something like : le(expr,bound) or ge(expr,bound)
    // in NodeBinary: parameter1 is expr to optimize, parameter2 is current bound
    void updateBound(long bound) override;

    long maxUpperBound() override;

    long minLowerBound() override;

    long computeScore(vec<int> &solution) override;

    void display(bool d) override;
};
}   // namespace Cosoco

#endif /* USE_XCSP3 */
#endif /* INTENSION_H */
