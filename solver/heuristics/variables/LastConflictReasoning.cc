#include "LastConflictReasoning.h"

using namespace Cosoco;


LastConflictReasoning::LastConflictReasoning(Solver &s, HeuristicVar *hv, int n) : HeuristicVar(s), hvar(hv), nVariables(n) {
    s.addObserverConflict(this);
}


Variable *LastConflictReasoning::select() {
    if(solver.statistics[restarts] < 2 && solver.nbSolutions == 0)
        return hvar->select();

    for(Variable *v : lcs)
        if(solver.isAssigned(v) == false)
            return v;

    return hvar->select();
}


void LastConflictReasoning::notifyConflict(Constraint *c, int level) {
    if(solver.decisionLevel() == 0)
        return;   // This is the end.
    lcs.clear();
    int i = 0;
    while(level - i >= 1 && i < nVariables) {
        lcs.push(solver.decisionVariableAtLevel(level - i));
        i++;
    }
}
