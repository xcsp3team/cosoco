#ifndef EQ_H
#define EQ_H

#include "constraints/Binary.h"

namespace Cosoco {


class EQ : public Binary {
   public:
    // Constructors
    int k;
    EQ(Problem &p, std::string n, Variable *xx, Variable *yy, int _k = 0);

    // filtering
    bool filter(Variable *x) override;

    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif /* EQ_H */
