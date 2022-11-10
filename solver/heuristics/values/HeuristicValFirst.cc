#include "solver/heuristics/values/HeuristicValFirst.h"

#include "solver/Solver.h"

using namespace Cosoco;

HeuristicValFirst::HeuristicValFirst(Solver &s) : HeuristicVal(s) { }


int HeuristicValFirst::select(Variable *x) { return x->domain.firstId(); }
