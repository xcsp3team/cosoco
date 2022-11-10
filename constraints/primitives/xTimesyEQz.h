#ifndef XTIMESYEQZ_H
#define XTIMESYEQZ_H

#include "constraints/Ternary.h"

namespace Cosoco {
class xTimesyEQz : public Ternary {
   public:
    int k;
    // Constructors
    xTimesyEQz(Problem &p, std::string n, Variable *xx, Variable *yy, Variable *zz);

    // filtering
    bool filter(Variable *x) override;
    bool instantiated(Variable *a, Variable *b);
    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif /* XTIMESYEQZ_H */
