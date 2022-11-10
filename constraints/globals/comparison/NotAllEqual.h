#ifndef COSOCO_NOTALLEQUAL_H
#define COSOCO_NOTALLEQUAL_H

#include "constraints/globals/GlobalConstraint.h"


namespace Cosoco {
class NotAllEqual : public GlobalConstraint {
   public:
    NotAllEqual(Problem &p, std::string n, vec<Variable *> &vars);

    // Filtering method, return false if a conflict occurs
    virtual bool filter(Variable *dummy) override;

    // Checking
    virtual bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco

#endif   // COSOCO_NOTALLEQUAL_H
