#ifndef COSOCO_RESTART_H
#define COSOCO_RESTART_H

#include <solver/Solver.h>

namespace Cosoco {
class Solver;
class Restart {
   public:
    Solver *solver;
    Restart(Solver *s) : solver(s) { }
    virtual bool isItTimeToRestart() = 0;   // Return true if the solver has to restart
    virtual void initialize()        = 0;   // Start from scratch policy
};
}   // namespace Cosoco
#endif   // COSOCO_RESTART_H
