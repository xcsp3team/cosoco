
#ifndef COSOCO_LOADFILE_H
#define COSOCO_LOADFILE_H

#include "XCSP3CoreParser.h"
#include "core/Problem.h"
#include "utils/CosocoCallbacks.h"


namespace Cosoco {


Problem *loadProblem(char *fileName, bool &optimize, int i2elimit) {
    CosocoCallbacks            cb(1, i2elimit);
    XCSP3Core::XCSP3CoreParser parser(&cb);
    parser.parse(fileName);   // parse the input file
    optimize = cb.optimizationProblem;
    return cb.problems[0];
}
}   // namespace Cosoco

#endif   // COSOCO_LOADFILE_H
