#include "LastConflictReasoning.h"

using namespace Cosoco;


LastConflictReasoning::LastConflictReasoning(Solver &s, std::unique_ptr<HeuristicVar> &&hv, int n)
    : HeuristicVar(s), hvar(std::move(hv)), nVariables(n), lastAssigned(nullptr), candidate(nullptr) {
    s.addObserverNewDecision(this);
    s.addObserverDeleteDecision(this);
}


Variable *LastConflictReasoning::select() {
    if(lcs.size() == 0) {
        if(lastAssigned == nullptr || lastAssigned->isAssigned())
            return hvar->select();
        lcs.push(lastAssigned);
        return lastAssigned;
    }
    // using one of the recorded variables?
    for(Variable *y : lcs)
        if(y->isAssigned() == false)
            return y;
    // leaving last reasoning mode?
    if(lcs.size() == nVariables || candidate == nullptr || candidate->isAssigned()) {
        lcs.clear();
        candidate = nullptr;
        return hvar->select();
    }
    // recording the candidate
    lcs.push(candidate);
    candidate = nullptr;
    return lcs.last();
    // return hvar->select();
}


void LastConflictReasoning::notifyDeleteDecision(Variable *x, int v, Solver &s) {
    if(lcs.size() == 0) {
        if(x != candidate)
            candidate = x;
    } else {
        if(lcs.size() < nVariables) {
            for(Variable *y : lcs)
                if(y == x)
                    return;
            candidate = x;
        }
    }
}

void LastConflictReasoning::notifyNewDecision(Variable *x, Solver &s) {
    if(solver.decisionLevel() == 0) {
        lcs.clear();
        candidate = nullptr;
    }
    if(lcs.size() == 0)
        lastAssigned = x;
}
