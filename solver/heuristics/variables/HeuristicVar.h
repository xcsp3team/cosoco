#ifndef HEURISTICVAR_H
#define HEURISTICVAR_H

#include "core/Variable.h"
namespace Cosoco {
class Solver;
class HeuristicVar {
   public:
    Solver &solver;
    HeuristicVar(Solver &s) : solver(s) { }

    virtual Variable *select() = 0;              // How to select a variable, return nullptr if none exist
    virtual bool      stop() { return false; }   //  Stop the search with this heuristic (only useful with LNS)
};
}   // namespace Cosoco


#endif /* HEURISTICVAR_H */
