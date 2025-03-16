#ifndef COSOCO_DISJUNCTIVE_H
#define COSOCO_DISJUNCTIVE_H

#include <constraints/globals/GlobalConstraint.h>

#include "constraints/Binary.h"


namespace Cosoco {
class Disjunctive : public Binary {
   protected:
    int  lx, ly;
    bool filterDomain(Variable *x, int lb, int ub);

   public:
    Disjunctive(Problem &p, std::string n, Variable *xx, Variable *yy, int ll1, int ll2);

    // filtering
    bool filter(Variable *x) override;

    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

class Disjunctive2D : public GlobalConstraint {
   protected:
    Variable *x1, *x2, *y1, *y2, *z;
    int       w1, w2, h1, h2;

   public:
    Disjunctive2D(Problem &p, std::string n, Variable *xx1, Variable *xx2, Variable *yy1, Variable *yy2, int ww1, int ww2,
                  int hh1, int hh2, Variable *z);

    // filtering
    bool filter(Variable *x) override;

    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

}   // namespace Cosoco

#endif   // COSOCO_DISJUNCTIVE_H
