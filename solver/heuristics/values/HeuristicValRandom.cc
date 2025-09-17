#include "solver/heuristics/values/HeuristicValRandom.h"

#include "solver/Solver.h"

using namespace Cosoco;

HeuristicValRandom::HeuristicValRandom(Solver &s) : HeuristicVal(s) { }

// Returns a random float 0 <= x < 1. Seed must never be 0.
static inline double drand(double &seed) {
    seed *= 1389796;
    int q = (int)(seed / 2147483647);
    seed -= (double)q * 2147483647;
    return seed / 2147483647;
}

// Returns a random integer 0 <= x < size. Seed must never be 0.
static inline int irand(double &seed, int size) { return static_cast<int>(drand(seed) * size); }


int HeuristicValRandom::select(Variable *x) {
    int pos = irand(solver.seed, x->size());
    return x->domain[pos];
}
