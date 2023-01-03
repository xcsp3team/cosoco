//
// Created by audemard on 02/01/23.
//

#ifndef COSOCO_BINPACKINGLOAD_H
#define COSOCO_BINPACKINGLOAD_H

#include "BinPacking.h"

namespace Cosoco {
class BinPackingLoad : public BinPacking {
   protected :
    SparseSet freeItems;
    vec<Variable*> loads;

   public :
    BinPackingLoad(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &_sizes, vec<Variable*> &_loads);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;

};
}   // namespace Cosoco

#endif   // COSOCO_BINPACKINGLOAD_H
