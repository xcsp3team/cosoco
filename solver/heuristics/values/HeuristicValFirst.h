#ifndef HEURISTICVALFIRST_H
#define HEURISTICVALFIRST_H

#include "HeuristicVal.h"

namespace Cosoco {
class HeuristicValFirst : public HeuristicVal {
   public:
    HeuristicValFirst(Solver &s);


    int select(Variable *x) override;
};
}   // namespace Cosoco


#endif /* HEURISTICVARFIRST_H */
