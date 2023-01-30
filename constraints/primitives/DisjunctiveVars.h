//
// Created by audemard on 28/01/23.
//

#ifndef COSOCO_DISJUNCTIVEVARS_H
#define COSOCO_DISJUNCTIVEVARS_H

#include "Constraint.h"
#include "GlobalConstraint.h"
namespace Cosoco {
class DisjunctiveVars : public GlobalConstraint {
   public:
    Variable *x1, *x2, *w1, *w2;
    DisjunctiveVars(Problem &p, std::string n, Variable *xx1, Variable *xx2, Variable *ww1, Variable *ww2);

    // filtering
    bool filter(Variable *x) override;

    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco

#endif   // COSOCO_DISJUNCTIVEVARS_H
