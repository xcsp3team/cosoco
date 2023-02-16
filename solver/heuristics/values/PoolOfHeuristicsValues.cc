//
// Created by audemard on 16/02/23.
//

#include "PoolOfHeuristicsValues.h"

#include "Solver.h"
using namespace Cosoco;

PoolOfHeuristicsValues::PoolOfHeuristicsValues(Solver &s) : HeuristicVal(s) {
    asgs   = new HeuristicValASGS(s);
    first  = new HeuristicValFirst(s);
    last   = new HeuristicValLast(s);
    occs   = new HeuristicValOccs(s);
    random = new HeuristicValRandom(s);
    heuristicForVariable.growTo(s.problem.variables.size(), first);
    selectHeuristics();
}

int PoolOfHeuristicsValues::select(Variable *x) { return heuristicForVariable[x->idx]->select(x); }

void PoolOfHeuristicsValues::selectHeuristics() {
    for(Variable *x : solver.problem.variables) {
        if(x->_name.rfind("jtp") == 0 || x->_name.rfind("cpu_loads") == 0) {
            heuristicForVariable[x->idx] = last;
        }
        if(x->_name.rfind("x") == 0) {
            heuristicForVariable[x->idx] = asgs;
        }
    }
}