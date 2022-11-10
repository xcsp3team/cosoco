//
// Created by audemard on 26/01/2021.
//

#ifndef COSOCO_DISTXYEQZ_H
#define COSOCO_DISTXYEQZ_H

#include <constraints/Ternary.h>
namespace Cosoco {

class DistXYeqZ : public Ternary {
   public:
    DistXYeqZ(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz);

    // filtering
    bool filter(Variable *x) override;
    bool instantiated(Variable *a, Variable *b);

    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
   private :
    vec<int> rx, ry, rzx, rzy;
    bool supportX(Variable *t, int v, int a, int b, int c);
    bool supportY(Variable *t, int v, int a, int b, int c);
    bool supportZ(Variable *t, int v, int a, int b, int c);

};
}   // namespace Cosoco


#endif   // COSOCO_DISTXYEQZ_H
