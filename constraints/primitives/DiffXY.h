#ifndef DIFFXY_H
#define DIFFXY_H

#include "constraints/Binary.h"

namespace Cosoco {

class DiffXY : public Binary {
   public:
    // Constructors
    DiffXY(Problem &p, std::string n, Variable *xx, Variable *yy);

    // filtering
    bool filter(Variable *x) override;

    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif /* DIFFXY_H */
