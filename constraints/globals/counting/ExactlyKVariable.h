#ifndef COSOCO_EXACTLYKVARIABLE_H
#define COSOCO_EXACTLYKVARIABLE_H

#include <constraints/globals/GlobalConstraint.h>

namespace Cosoco {

class ExactlyKVariable : public GlobalConstraint {
    vec<Variable *> list;   // Put list in a dedicated array (do not use scope)
    Variable *      k;
    int             value;
    int             positionOfKInList;   // -1 if not present

   public:
    ExactlyKVariable(Problem &p, std::string n, vec<Variable *> &vars, Variable *k, int val);

    // Filtering method, return false if a conflict occurs
    virtual bool filter(Variable *x) override;

    // Checking
    virtual bool isSatisfiedBy(vec<int> &tuple) override;

    virtual bool isCorrectlyDefined() override;
};
}   // namespace Cosoco

#endif   // COSOCO_EXACTLYKVARIABLE_H
