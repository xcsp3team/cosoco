#include "HeuristicValASGS.h"

#include "solver/Solver.h"

using namespace Cosoco;

HeuristicValASGS::HeuristicValASGS(Solver &s) : HeuristicVal(s) { }


int HeuristicValASGS::select(Variable *x) {
    int tmp = x->domain[0];
    for(int idv : x->domain) {
        if(x->domain.nAssignments[tmp] > x->domain.nAssignments[idv])
            tmp = idv;
    }
    return tmp;
}
