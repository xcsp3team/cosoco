//
// Created by audemard on 18/02/2026.
//

#ifndef COSOCO_IFF_H
#define COSOCO_IFF_H
#include "Binary.h"
#include "CosocoCallbacks.h"

namespace Cosoco {


class Iff : public Binary {
    BasicNode *n1;
    BasicNode *n2;

   public:
    Iff(Problem &p, std::string n, Variable *x1, Variable *x2, BasicNode *n1, BasicNode *n2);
    bool filter(Variable *x) override;
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco

#endif   // COSOCO_IFF_H
