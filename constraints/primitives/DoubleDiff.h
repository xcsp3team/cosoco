//
// Created by audemard on 15/10/2025.
//

#ifndef COSOCO_DOUBLEDIFF_H
#define COSOCO_DOUBLEDIFF_H
#include "Constraint.h"


namespace Cosoco {


class DoubleDiff : public Constraint {
   protected:
    int sentinel1, sentinel2;

    int  findAnotherSentinel();
    bool enforceSentinel(int sentinel);

   public:
    // Constructors
    DoubleDiff(Problem &p, std::string n, Variable *xx1, Variable *xx2, Variable *yy1, Variable *yy2);

    // filtering
    bool filter(Variable *x) override;

    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif   // COSOCO_DOUBLEDIFF_H
