//
// Created by audemard on 15/10/2025.
//

#ifndef COSOCO_XOR_H
#define COSOCO_XOR_H
#include "GlobalConstraint.h"


namespace Cosoco {


class Xor : public GlobalConstraint {
   protected:
    Variable *sentinel1, *sentinel2;

    Variable *findAnotherSentinel();
    bool      enforceSentinel(Variable *sentinel);

   public:
    // Constructors
    Xor(Problem &p, std::string n, vec<Variable *> &vars);

    // filtering
    bool filter(Variable *x) override;

    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

class XeqXor : public GlobalConstraint {
   protected:
    Variable       *x;
    vec<Variable *> vars;
    Variable       *sentinel1, *sentinel2;

    bool enforceSentinel(Variable *sentinel, int value);
    int  toBeRemoved(Variable *sentinel);

   public:
    // Constructors
    XeqXor(Problem &p, std::string n, vec<Variable *> &vars, Variable *xx);
    Variable *findAnotherSentinel(Variable *other);

    // filtering
    bool filter(Variable *x) override;
    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};


}   // namespace Cosoco


#endif   // COSOCO_XOR_H
