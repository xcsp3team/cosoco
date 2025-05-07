#include "solver/heuristics/values/HeuristicValLast.h"

#include "solver/Solver.h"

using namespace Cosoco;


HeuristicValLast::HeuristicValLast(Solver &s) : HeuristicVal(s) { std::cout << "ici\n"; }


int HeuristicValLast::select(Variable *x) { return x->domain.lastId(); }
