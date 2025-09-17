//
// Created by audemard on 16/11/22.
//

#ifndef COSOCO_HEURISTICVALOCCS_H
#define COSOCO_HEURISTICVALOCCS_H

#include "HeuristicVal.h"
#include "Map.h"

namespace Cosoco {
struct IntHash {
    uint32_t operator()(int v) const { return static_cast<uint32_t>(v); }
};
class HeuristicValOccs : public HeuristicVal {
    vec<Map<int, int, IntHash> > nbOccurrences;   // the occurences for each array of variables
    vec<vec<int> >               elements;        // the element sin the map
    vec<uint64_t>                lastConflict;    // last time we updateed the array

   public:
    explicit HeuristicValOccs(Solver &s);
    int  select(Variable *x) override;
    void updateOccurrences(int array);
};
}   // namespace Cosoco

#endif   // COSOCO_HEURISTICVALOCCS_H
