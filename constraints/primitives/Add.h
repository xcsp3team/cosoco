#ifndef ADD_H
#define ADD_H


#include "Ternary.h"
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
    bool isSatisfiedBy(vec<int> &tuple) override;
};

class Add3 : public Ternary {
    static bool tooLarge(int size1, int size2) { return size1 > 1 && size2 > 1 && size1 * static_cast<double>(size2) > 200; }

   public:
    Add3(Problem &p, std::string n, Variable *x, Variable *y, Variable *z);
    // filtering
    bool filter(Variable *x) override;

    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

}   // namespace Cosoco


#endif /* LT_H */
