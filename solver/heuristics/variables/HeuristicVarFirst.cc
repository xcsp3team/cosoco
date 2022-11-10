#include "HeuristicVarFirst.h"

using namespace Cosoco;


HeuristicVarFirst::HeuristicVarFirst(Solver &s) : HeuristicVar(s) { }


Variable *HeuristicVarFirst::select() {
    int idx      = solver.decisionVariables[0]->idx;
    int position = 0;
    for(int i = 1; i < solver.decisionVariables.size(); i++) {
        if(solver.decisionVariables[i]->size() == 1)
            return solver.decisionVariables[i];
        if(solver.decisionVariables[i]->idx < idx) {
            idx      = solver.decisionVariables[i]->idx;
            position = i;
        }
    }
    return solver.decisionVariables[position];
}
