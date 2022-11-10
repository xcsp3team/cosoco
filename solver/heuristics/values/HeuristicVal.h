#ifndef HEURISTICVAL_H
#define HEURISTICVAL_H
#include "core/Variable.h"

namespace Cosoco {
class Solver;
class HeuristicVal {
   public:
    Solver &solver;
    HeuristicVal(Solver &s) : solver(s) { }

    virtual int select(Variable *x) = 0;   // How to select a variable
};
}   // namespace Cosoco


#endif /* HEURISTICVAR_H */
