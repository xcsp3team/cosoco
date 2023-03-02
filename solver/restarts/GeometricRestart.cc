//
// Created by audemard on 12/12/2015.
//

#include "GeometricRestart.h"


bool Cosoco::GeometricRestart::isItTimeToRestart() {
    if(solver->conflicts > limit) {
        numberOfConflicts *= factor;
        limit += numberOfConflicts;
        if(init) {
            numberOfConflicts = 100;
            limit             = solver->conflicts + 100;
            init              = false;
        }
        return true;
    }
    return false;
}
