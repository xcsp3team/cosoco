//
// Created by audemard on 09/04/24.
//

#ifndef COSOCO_REIFICATION_H
#define COSOCO_REIFICATION_H


#include "constraints/Ternary.h"

namespace Cosoco {
class ReifLE : public Ternary {
   public:
    ReifLE(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz);
    bool filter(Variable *x) override;
    bool isSatisfiedBy(vec<int> &tuple) override;
};


class ReifLT : public Ternary {
   public:
    ReifLT(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz);
    bool filter(Variable *x) override;
    bool isSatisfiedBy(vec<int> &tuple) override;
};

class ReifEQ : public Ternary {
   public:
    int residue;
    ReifEQ(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz);
    bool filter(Variable *x) override;
    bool isSatisfiedBy(vec<int> &tuple) override;
};

class ReifNE : public Ternary {
   public:
    int residue;
    ReifNE(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz);
    bool filter(Variable *x) override;
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif   // COSOCO_REIFICATION_H
