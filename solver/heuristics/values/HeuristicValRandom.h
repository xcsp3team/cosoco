#ifndef HEURISTICVALRANDOM_H
#define HEURISTICVALRANDOM_H

#include "HeuristicVal.h"

namespace Cosoco {
class HeuristicValRandom : public HeuristicVal {
   public:
    HeuristicValRandom(Solver &s);


    virtual int select(Variable *x) override;
};
}   // namespace Cosoco


#endif /* HEURISTICVARFIRST_H */
