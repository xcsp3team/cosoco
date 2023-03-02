#ifndef COSOCO_GEOMETRICRESTART_H
#define COSOCO_GEOMETRICRESTART_H


#include "Restart.h"

namespace Cosoco {
class GeometricRestart : public Restart {
    unsigned int numberOfConflicts;
    unsigned int limit;
    int          nbC;
    double       factor;
    bool         init;

   public:
    GeometricRestart(Solver *s, int nof, double f) : Restart(s), numberOfConflicts(nof), factor(f) { init = false; }
    GeometricRestart(Solver *s) : Restart(s), numberOfConflicts(100), limit(100), nbC(1), factor(1.1) { init = false; }
    bool isItTimeToRestart() override;

    void initialize() override { init = true; }
};
}   // namespace Cosoco


#endif   // COSOCO_GEOMETRICRESTART_H
