#ifndef COSOCO_UNARY_H
#define COSOCO_UNARY_H

#include "constraints/Binary.h"
#include "constraints/Constraint.h"

namespace Cosoco {

class Unary : public Constraint {
   public:
    Variable *x;
    bool      done;
    bool      areSupports;
    vec<int>  values;

    Unary(Problem &p, std::string n, Variable *xx, const vec<int> &vals, bool areS);
    State status() override;
    void  reinitialize() override;
    bool  filter(Variable *x) override;
    bool  isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif   // COSOCO_UNARY_H
