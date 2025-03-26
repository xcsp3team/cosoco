//
// Created by audemard on 26/03/25.
//

#ifndef COSOCO_MATCHER_H
#define COSOCO_MATCHER_H

#include "Constraint.h"
namespace Cosoco {
class Matcher {
   protected:
    Constraint *c;                              // The associated constraint
    int         minValue, maxValue, interval;   // min, max and interval
    int        *val2var, *var2val;
    
   public:
    Matcher(Constraint *c);
    bool findMaximumMatching();
    bool findMatchingFor(int x);
};
}   // namespace Cosoco

#endif   // COSOCO_MATCHER_H
