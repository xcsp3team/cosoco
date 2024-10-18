//
// Created by audemard on 18/10/24.
//

#ifndef COSOCO_PROFILING_H
#define COSOCO_PROFILING_H
#include "solver/Solver.h"
namespace Cosoco {
class ConstraintData {
   public:
    double totalTime    = 0;
    long   nbCalls      = 0;
    long   uselessCalls = 0;
};


class Profiling {
    Solver                               *solver;
    std::map<std::string, ConstraintData> constraintsData;
    double                                currentTime = 0;

   public:
    explicit Profiling(Solver *s);

    void initialize();

    bool beforeConstraintCall(Constraint *c);

    bool afterConstraintCall(Constraint *c, int nbDeletedVariables);

    void display();
};

}   // namespace Cosoco

#endif   // COSOCO_PROFILING_H
