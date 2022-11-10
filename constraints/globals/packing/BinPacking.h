//
// Created by audemard on 08/02/2022.
//

#ifndef COSOCO_BINPACKING_H
#define COSOCO_BINPACKING_H

#include "constraints/globals/GlobalConstraint.h"


namespace Cosoco {

class BinPacking : public GlobalConstraint {
   public:
    vec<int> sizes;
    BinPacking(Problem &p, std::string n, vec<Variable *> vars, vec<int> szs, int uB);

    virtual bool isCorrectlyDefined() override;

    // Filtering method, return false if a conflict occurs
    virtual bool filter(Variable *x) override;

    // Checking
    virtual bool isSatisfiedBy(vec<int> &tuple) override;

   protected:



};
}
#endif   // COSOCO_BINPACKING_H