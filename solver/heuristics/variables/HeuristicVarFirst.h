#ifndef HEURISTICVARFIRST_H
#define HEURISTICVARFIRST_H

#include "HeuristicVar.h"
#include "solver/Solver.h"

namespace Cosoco {
class HeuristicVarFirst : public HeuristicVar {
   public:
    HeuristicVarFirst(Solver &s);


    virtual Variable *select() override;
};
}   // namespace Cosoco


#endif /* HEURISTICVARFIRST_H */
