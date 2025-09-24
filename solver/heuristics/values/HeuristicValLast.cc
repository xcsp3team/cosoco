#include "solver/heuristics/values/HeuristicValLast.h"

#include "solver/Solver.h"

using namespace Cosoco;


HeuristicValLast::HeuristicValLast(Solver &s) : HeuristicVal(s) { }


int HeuristicValLast::select(Variable *x) { return x->domain.lastId(); }
