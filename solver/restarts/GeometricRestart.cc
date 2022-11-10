//
// Created by audemard on 12/12/2015.
//

#include "GeometricRestart.h"


bool Cosoco::GeometricRestart::isItTimeToRestart() {
    if(solver->conflicts > limit) {
        numberOfConflicts *= factor;
        limit += numberOfConflicts;
        return true;
    }
    return false;
}
