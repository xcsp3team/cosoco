#ifndef COSOCO_GEOMETRICRESTART_H
#define COSOCO_GEOMETRICRESTART_H


#include "Restart.h"

namespace Cosoco {
class GeometricRestart : public Restart {
    unsigned int numberOfConflicts;
    unsigned int limit {};
    int          nbC {};
    double       factor;

   public:
    GeometricRestart(Solver *s, int nof, double f) : Restart(s), numberOfConflicts(nof), factor(f) { }
    explicit GeometricRestart(Solver *s) : Restart(s), numberOfConflicts(100), limit(100), nbC(1), factor(1.1) { }
    bool isItTimeToRestart() override;

    void initialize() override {
        numberOfConflicts = 100;
        limit             = solver->conflicts + 100;
    }
};
}   // namespace Cosoco


#endif   // COSOCO_GEOMETRICRESTART_H
