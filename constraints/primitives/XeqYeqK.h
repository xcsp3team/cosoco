//
// Created by audemard on 01/02/2021.
//

#ifndef COSOCO_XEQYEQK_H
#define COSOCO_XEQYEQK_H

#include "constraints/Binary.h"
#include "solver/Solver.h"


namespace Cosoco {

class XeqYeqK : public Binary {
   public:
    int k;
    // Constructors
    XeqYeqK(Problem &p, std::string n, Variable *xx, Variable *yy, int k);

    // filtering
    bool filter(Variable *x) override;

    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco



#endif   // COSOCO_XEQYEQK_H
