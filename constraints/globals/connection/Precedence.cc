#include "Precedence.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool Precedence::isSatisfiedBy(vec<int> &tuple) {
    int i = 0, prev = tuple.firstOccurrenceOf(values[0]);
    while(i < k - 1 && prev != -1) {
        i++;
        int curr = tuple.firstOccurrenceOf(values[i]);
        if(curr != -1 && curr < prev)
            return false;
        prev = curr;
    }
    if(prev == -1) {
        if(covered)
            return false;
        // we check that all other values are absent
        for(int j = i + 1; j < k; j++)
            if(tuple.firstOccurrenceOf(values[j]) != -1)
                return false;
        return true;
    }
    return true;
}


bool Precedence::isCorrectlyDefined() { return true; }


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

void Precedence::notifyDeleteDecision(Variable *x, int v, Solver &s) { reinit = true; }

bool Precedence::filter(Variable *x) {
    if(solver->decisionLevel() == 0 && solver->conflicts == 0) {
        for(int i = 1; i < k; i++)
            for(int j = 0; j < i; j++) solver->delVal(scope[j], values[i]);
    }

    if(reinit) size = k;

    assert(size > 1);
    for(int i = 0; i < size; i++) {
        int v = values[i];
        int j = reinit ? i : firsts[i];   // i because of the initial removals
        while(j < r) {
            if(scope[j]->containsValue(v)) {
                if(i == 0 || firsts[i - 1] < j)
                    break;   // because valid
                if(solver->delVal(scope[j], v) == false)
                    return false;
            }
            j++;
        }
        firsts[i] = j;
        if(j == r) {
            if(covered)
                return false;
            // we remove all subsequent values
            for(int k = i + 1; k < size; k++) {
                int w = values[k];
                for(int l = reinit ? k : firsts[k]; l < r; l++)
                    if(solver->delVal(scope[l], w) == false)
                        return false;
            }
            size = i;
        }
    }
    for(int i = size - 1; i > 0; i--) {
        assert(scope[firsts[i]]->containsValue(values[i]) && firsts[i] > firsts[i - 1]);
        if(scope[firsts[i]]->size() > 1 || scope[firsts[i - 1]]->size() == 1)
            continue;
        if(firsts[i] - firsts[i - 1] > dist)
            continue;
        bool absent = true;
        for(int j = firsts[i - 1] + 1; absent && j < firsts[i]; j++)
            if(scope[j]->containsValue(values[i - 1]))
                absent = false;
        if(absent)
            if(solver->assignToVal(scope[firsts[i - 1]],values[i - 1]) == false)
                return false;
    }
    int i = size - 1;
    while(i > 0 && scope[firsts[i]]->size() == 1 && scope[firsts[i - 1]]->size() == 1) i--;
    size = i + 1;
    if(size <= 1)
        solver->entail(this);
    return true;
}



//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

Precedence::Precedence(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &vs)
    : GlobalConstraint(p, n, "Precedence", vars) {
    vs.copyTo(values);
    // control((!covered || list.length > values.length) && values.length > 1);
    r       = scope.size();
    k       = values.size();
    covered = false;   // TODO
    firsts.growTo(k, 0);
}
