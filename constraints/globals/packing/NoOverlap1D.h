//
// Created by audemard on 14/11/2026.
//

#ifndef COSOCO_NOOVERLAP1D_H
#define COSOCO_NOOVERLAP1D_H
#include "GlobalConstraint.h"

namespace Cosoco {

struct Segment {
    int start, end, id;
};

class NoOverlap1D : public GlobalConstraint {
    vec<int> lengths;

    bool timeTabling();

   public:
    NoOverlap1D(Problem &p, vec<Variable *> &vars, vec<int> &_lengths);
    bool isCorrectlyDefined() override;


    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco

#endif   // COSOCO_NOOVERLAP1D_H
