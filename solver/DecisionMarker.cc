#include "DecisionMarker.h"

#include <cmath>

#include "Solver.h"

using namespace Cosoco;


DecisionMarker::DecisionMarker(Solver *s) : solver(s) {
    assert(false);
    unsigned int n1 = (int)ceil(log2(s->problem.nbVariables()));
    unsigned int n2 = (int)ceil(log2(s->problem.maximumDomainSize()));

    if(n1 + n2 <= sizeof(long) * 8)
        assert(false);// Number of bits....

    OFFSET = (int)pow(2, n2 + 1);   // +1 because 0 excluded ???
    s->addObserverNewDecision(this);
    s->addObserverDeleteDecision(this);
}


void DecisionMarker::notifyNewDecision(Variable *x, Solver &s) { currentBranch.push(getPositiveDecisionFor(x, x->valueId())); }


void DecisionMarker::notifyDeleteDecision(Variable *x, int v, Solver &s) {
    int current = getPositiveDecisionFor(x, x->domain.toIdv(v));
    assert(currentBranch.firstOccurrenceOf(current) >= 0);
    currentBranch.cut(currentBranch.firstOccurrenceOf(current) + 1);
    assert(currentBranch.last() == current);
    currentBranch.last() = -currentBranch.last();
    assert(currentBranch.contains(current) == false);
}


void DecisionMarker::notifyFullBacktrack() { currentBranch.clear(); }


inline int DecisionMarker::depth() { return currentBranch.size(); }


Variable *DecisionMarker::getVariableIn(int number) { return solver->problem.variables[abs(number) / OFFSET]; }


inline int DecisionMarker::getIndexIn(int number) { return abs(number) % OFFSET - 1; }


inline int DecisionMarker::getPositiveDecisionFor(Variable *x, int idv) { return 1 + idv + OFFSET * x->idx; }


inline int DecisionMarker::getNegativeDecisionFor(Variable *x, int idv) { return -getPositiveDecisionFor(x, idv); }


bool DecisionMarker::generateNogoodsFromRestarts() {   // See Nogood Recording From Restarts, Lecoutre et al. Ijcai07
    return true;
}

std::string DecisionMarker::getStringFor(int dec) {
    return getVariableIn(dec)->_name + (dec > 0 ? "=" : "!=") + std::to_string(getIndexIn(dec));
}


void DecisionMarker::display() {
    for(int i = 0; i < currentBranch.size(); i++) {
        std::cout << getStringFor(currentBranch[i]) << " ";
    }
    std::cout << std::endl;
}