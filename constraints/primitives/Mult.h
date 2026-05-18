//
// Created by audemard on 30/01/2026.
//

#ifndef COSOCO_MULT_H
#define COSOCO_MULT_H
#include "Ternary.h"

namespace Cosoco {
class Mult3EQ : public Ternary {
   protected:
    static bool tooLarge(int size1, int size2) { return size1 > 1 && size2 > 1 && size1 * static_cast<double>(size2) > 1000000; }

    vec<int> rx;
    vec<int> ry;
    vec<int> rzx;
    vec<int> rzy;

    bool reviseGE(Variable *x, Variable *y, int k);
    bool reviseLE(Variable *x, Variable *y, int k);
    bool enforceMulGE(Variable *x, Variable *y, int k);
    bool enforceMulLE(Variable *x, Variable *y, int k);

   public:
    Mult3EQ(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz);

    // filtering
    bool filter(Variable *x) override;

    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif   // COSOCO_MULT_H
