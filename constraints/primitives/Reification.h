//
// Created by audemard on 09/04/24.
//

#ifndef COSOCO_REIFICATION_H
#define COSOCO_REIFICATION_H


#include "constraints/Ternary.h"

namespace Cosoco {
class ReifLE : public Ternary {
   public:
    int k;
    // Constructors
    ReifLE(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz);

    // filtering
    bool filter(Variable *x) override;
    bool instantiated(Variable *a, Variable *b);
    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};


class ReifLT : public Ternary {
   public:
    int k;
    // Constructors
    ReifLT(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz);

    // filtering
    bool filter(Variable *x) override;
    bool instantiated(Variable *a, Variable *b);
    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};


}   // namespace Cosoco


#endif   // COSOCO_REIFICATION_H
