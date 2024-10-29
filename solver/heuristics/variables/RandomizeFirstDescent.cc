#include "RandomizeFirstDescent.h"

using namespace Cosoco;


RandomizeFirstDescent::RandomizeFirstDescent(Solver &s, std::unique_ptr<HeuristicVar> &&hv)
    : HeuristicVar(s), firstDescent(true), hvar(std::move(hv)) {
    s.addObserverConflict(this);
}


Variable *RandomizeFirstDescent::select() {
    if(firstDescent) {
        int pos = irand(solver.seed, solver.problem.variables.size());

        if(solver.problem.variables[pos]->isAssigned() == false)
            return solver.problem.variables[pos].get();
    }
    return hvar->select();
}


void RandomizeFirstDescent::notifyConflict(Constraint *c, int level) { firstDescent = false; }
