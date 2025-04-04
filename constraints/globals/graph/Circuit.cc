#include "Circuit.h"

#include "core/domain/DomainRange.h"
#include "solver/Solver.h"
using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool Circuit::isSatisfiedBy(vec<int> &tuple) {
    if(AllDifferentAC::isSatisfiedBy(tuple) == false)
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
    if(AllDifferentAC::filter(dummy) == false)
        return false;
    if(unassignedVariablesIdx.isEmpty())
        return true;

    int minimalCircuitLength = 0;
    heads.clear();
    int circuitNode = -1;
    pheads.fill();
    int nArcs = 0;
    pred.fill(false);
    int nSelfLoops = 0;
    for(int idx = 0; idx < scope.size(); idx++) {
        if(scope[idx]->containsValue(idx) == false)
            minimalCircuitLength++;
        if(scope[idx]->size() == 1) {
            // if (pheads.contains(i)) pheads.remove(i);
            int j = scope[idx]->value();
            if(idx == j) {
                nSelfLoops++;
                continue;   // because auto-loop
            }
            nArcs++;
            // if (pheads.contains(j)) pheads.remove(j);
            if(pred[idx] == false)
                heads.add(idx);
            if(pred[j] == true)
                return false;   // fail because two predecessors
            pred[j] = true;
            if(heads.contains(j)) {
                heads.del(j);
                if(heads.isEmpty())   // it means that we have a closed circuit
                    circuitNode = idx;
            }
        }
    }
    if(nSelfLoops == scope.size())   // TODO: we should prune when all but two variables are self loops (using three
                                     // residues?)
        return false;

    if(circuitNode != -1) {
        if(!heads.isEmpty())
            return false;   // because a closed circuit and a separate chain
        set.clear();
        int i = circuitNode;
        set.add(i);
        while(true) {
            i = scope[i]->value();
            if(i == circuitNode)
                break;
            set.add(i);
        }
        if(set.size() < nArcs)
            return false;
        for(int k = 0; k < scope.size(); k++)
            if(!set.contains(k) && solver->assignToVal(scope[k], k) == false)
                return false;
        return true;   // TODO : entail
    }
    int cnt = 0;
    tmp.fill(false);
    for(int i : heads) {
        if(tmp[i])
            continue;   // because it is a head that has just been reached from another head after filtering
        assert(scope[i]->size() == 1 && scope[i]->value() != i);
        int j    = scope[i]->value();
        int head = i;
        set.clear();
        // set.add(i); // i belongs to the circuit
        while(true) {
            int before = scope[j]->size();
            if(set.contains(j))
                return false;                          // because two times the same value (and too short circuit)
            if(solver->delVal(scope[j], j) == false)   // because self-loop not possible for j
                return false;
            if(set.size() + 2 < minimalCircuitLength)
                if(solver->delVal(scope[j], head) == false)
                    return false;

            for(int v1 : set)
                if(solver->delVal(scope[j], v1) == false)
                    return false;
            if(pred[j])
                cnt++;
            if(scope[j]->size() > 1)
                break;
            set.add(j);              // j belongs to the circuit
            j = scope[j]->value();   // we know that the *new value of j* is different from the previous one
            if(before > 1) {
                // if (pheads.contains(j)) pheads.remove(j);
                if(heads.contains(j))
                    tmp[j] = true;
            }
            if(j == head) {   // closed circuit
                for(int k = 0; k < scope.size(); k++)
                    if(k != head && !set.contains(k) && solver->assignToVal(scope[k], k) == false)
                        return false;
                return true;   // TODO entail();
            }
        }
    }
    if(cnt < nArcs)
        return false;

    return true;
}

/*bool Circuit::filter(Variable *dummy) {
    if(AllDifferentAC::filter(dummy) == false)
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
*/

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

Circuit::Circuit(Problem &p, std::string n, vec<Variable *> &vars) : AllDifferentAC(p, n, vars) {
    type = "Circuit";
    set.setCapacity(vars.size(), false);
    tmp.growTo(scope.size());
    pred.growTo(scope.size());
    heads.setCapacity(vars.size(), false);
    pheads.setCapacity(vars.size(), false);
}
/*Circuit::Circuit(Problem &p, std::string n, vec<Variable *> &vars) : AllDifferentAC(p, n, vars) {
    type = "Circuit";
    set.setCapacity(vars.size(), false);
    tmp.growTo(scope.size());
}*/
