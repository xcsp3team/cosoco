#ifndef LT_H
#define LT_H


#include "constraints/Binary.h"

namespace Cosoco {

class Lt : public Binary {
   public:
    int k;
    // Constructors
    Lt(Problem &p, std::string n, Variable *xx, Variable *yy, int k = 0);

    // filtering
    bool filter(Variable *x) override;

    // checking
    virtual bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif /* LT_H */
