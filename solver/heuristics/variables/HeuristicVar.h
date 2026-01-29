#ifndef HEURISTICVAR_H
#define HEURISTICVAR_H

#include "core/Variable.h"
namespace Cosoco {
class Solver;
class HeuristicVar {
   protected:
    bool      freezed;
    Variable *secondBest = nullptr;

   public:
    virtual ~HeuristicVar() = default;
    Solver &solver;
    explicit HeuristicVar(Solver &s) : freezed(false), solver(s) { }

    virtual Variable *select() = 0;   // How to select a variable, return nullptr if none exist
    virtual bool      stop() {        //  Stop the search with this heuristic
        freezed = true;
        return false;
    }

    virtual bool start() {   // start the search with this heuristic
        freezed = false;
        return true;
    }
    virtual void penalize(Variable *var) { }   // Penalize impactless variables
};
}   // namespace Cosoco


#endif /* HEURISTICVAR_H */
