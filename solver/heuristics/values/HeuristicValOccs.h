//
// Created by audemard on 16/11/22.
//

#ifndef COSOCO_HEURISTICVALOCCS_H
#define COSOCO_HEURISTICVALOCCS_H
#include "HeuristicVal.h"


namespace Cosoco {
class HeuristicValOccs : public HeuristicVal {
    vec<vec<int> > nbOccurrences;   // the occurences for each array of variables
    vec<uint64_t>  lastConflict;    // last time we updateed the array

   public:
    HeuristicValOccs(Solver &s);
    int  select(Variable *x) override;
    void updateOccurrences(int array);
};
}   // namespace Cosoco

#endif   // COSOCO_HEURISTICVALOCCS_H
