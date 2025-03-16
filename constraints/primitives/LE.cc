#include "LE.h"

#include "solver/Solver.h"

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool Le::isSatisfiedBy(vec<int> &tuple) { return tuple[0] + k <= tuple[1]; }

//----------------------------------------------
// Filtering
//----------------------------------------------

bool Le::filter(Variable *dummy) { return solver->enforceLE(x, y, k); }

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

Le::Le(Problem &p, std::string n, Variable *xx, Variable *yy, int kk) : Binary(p, n, xx, yy), k(kk) { type = "X + k  <= y"; }
