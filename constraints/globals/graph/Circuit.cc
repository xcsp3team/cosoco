#include "Circuit.h"

#include "core/domain/DomainRange.h"
#include "solver/Solver.h"
using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool Circuit::isSatisfiedBy(vec<int> &tuple) {
    if(AllDifferent::isSatisfiedBy(tuple) == false)
        return false;
    int nLoops = 0, first = -1;
    for(int i = 0; i < tuple.size(); i++)
        if(tuple[i] == i)
            nLoops++;
        else if(first == -1)
            first = i;
    if(nLoops == tuple.size())
        return false;   // because no circuit at all

    tmp.fill(false);

    int i = first, size = 0;
    while(!tmp[tuple[i]]) {
        if(tuple[i] == i)
            return false;   // because badly formed circuit
        tmp[tuple[i]] = true;
        i             = tuple[i];
        size++;
    }
    return size == tuple.size() - nLoops;
}


bool Circuit::isCorrectlyDefined() {
    for(Variable *x : scope)
        if(x->minimum() < 0 || x->maximum() >= scope.size())
            return false;
    return true;
}

//----------------------------------------------
// Filtering
//----------------------------------------------

bool Circuit::filter(Variable *dummy) {
    if(AllDifferent::filter(dummy) == false)
        return false;

    if(unassignedVariablesIdx.isEmpty())
        return true;
    int minimalCircuitLength = 0;
    for(int i = 0; i < scope.size(); i++)
        if(scope[i]->containsValue(i) == false)
            minimalCircuitLength++;

    tmp.fill(false);

    int nSelfLoops = 0;
    for(int i = 0; i < scope.size(); i++) {
        if(scope[i]->size() > 1 || tmp[i])
            continue;
        int j = scope[i]->value();
        if(i == j) {
            nSelfLoops++;
            continue;   // because self-loop
        }
        set.clear();
        set.add(i);                                // i belongs to the circuit
        if(solver->delVal(scope[j], j) == false)   // because self-loop not possible for j
            return false;
        while(set.size() + 1 < minimalCircuitLength) {
            for(int v : set)
                if(solver->delVal(scope[j], v) == false)   // because we cannot close the circuit now (it would be too short)
                    return false;

            if(scope[j]->size() > 1)
                break;
            tmp[j] = true;
            if(set.contains(j))
                return false;                          // because two times the same value (and too short circuit)
            set.add(j);                                // j belongs to the circuit
            j = scope[j]->value();                     // we know that the *new value of j* is different from the previous one
            if(solver->delVal(scope[j], j) == false)   // because self-loop not possible for j
                return false;
        }
        if(scope[j]->size() == 1 && scope[j]->value() == set[0]) {
            for(int k = 0; k < scope.size(); k++)
                if(j != k && !set.contains(k) && solver->assignToVal(scope[k], k) == false)
                    return false;
            solver->entail(this);
            return true;
        }
    }
    if(nSelfLoops == scope.size())   // TODO: we should prune when all but two variables are self loops
        return false;
    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

Circuit::Circuit(Problem &p, std::string n, vec<Variable *> &vars) : AllDifferent(p, n, vars) {
    type = "Circuit";
    set.setCapacity(vars.size(), false);
    tmp.growTo(scope.size());
}
