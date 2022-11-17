#ifndef HEURISTICVALFIRST_H
#define HEURISTICVALFIRST_H

#include "HeuristicVal.h"

namespace Cosoco {
class HeuristicValFirst : public HeuristicVal {
   public:
    explicit HeuristicValFirst(Solver &s);


    int select(Variable *x) override;
};
}   // namespace Cosoco


#endif /* HEURISTICVALFIRST_H */
