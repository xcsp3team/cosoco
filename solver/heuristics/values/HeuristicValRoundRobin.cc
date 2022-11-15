//
// Created by audemard on 15/11/22.
//

#include "HeuristicValRoundRobin.h"

#include "solver/Solver.h"

using namespace Cosoco;

HeuristicValRoundRobin::HeuristicValRoundRobin(Solver &s) : HeuristicVal(s), rnd(HeuristicValRandom(s)) { }

int HeuristicValRoundRobin::select(Variable *x) {
    unsigned int curRestart = solver.statistics[restarts];
    if(curRestart % 3 == 0)
        return x->domain.firstId();
    if(curRestart % 3 == 1)
        return x->domain.lastId();
    return rnd.select(x);
}
