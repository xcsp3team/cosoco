#ifndef LE_H
#define LE_H

#include "constraints/Binary.h"

namespace Cosoco {

class Le : public Binary {
   public:
    int k;
    // Constructors
    Le(Problem &p, std::string n, Variable *xx, Variable *yy, int k = 0);

    // filtering
    bool filter(Variable *x) override;

    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif /* LE_H */
