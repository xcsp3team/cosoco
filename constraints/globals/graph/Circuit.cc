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
    int nLoops = 0;
    for(int i = 0; i < tuple.size(); i++)
        if(tuple[i] == i)
            nLoops++;

    if(nLoops == tuple.size())
        return false;

    int i = 0;
    while(i < scope.size() && tuple[i] == i) i++;

    SparseSet s;
    s.setCapacity(scope.size(), false);
    while(tuple[i] != i && s.contains(tuple[i]) == false) {
        s.add(tuple[i]);
        i = tuple[i];
    }
    return s.size() == (tuple.size() - nLoops);
}


//----------------------------------------------
// Filtering
//----------------------------------------------

bool Circuit::filter(Variable *x) {
    if(AllDifferent::filter(x) == false)
        return false;


    int minimalCircuitLength = 0;
    for(int i = 0; i < scope.size(); i++)
        if(scope[i]->containsIdv(i) == false)
            minimalCircuitLength++;


    for(int i = 0; i < scope.size(); i++) {
        if(scope[i]->size() == 1) {
            int j = scope[i]->value();
            if(i == j)
                continue;   // because self-loop
            set.clear();
            set.add(i);
            if(solver->delIdv(scope[j], j) == false)
                return false;

            while(set.size() + 1 < minimalCircuitLength) {
                for(int idv : set) {
                    if(solver->delIdv(scope[j], idv) == false)
                        return false;
                }
                if(scope[j]->size() == 1) {
                    set.add(j);
                    j = scope[j]->value();   // we know for sure here that the new value of j is different from the previous one
                    if(solver->delIdv(scope[j], j) == false)
                        return false;
                } else
                    break;
            }
        }
    }
    return true;
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

Circuit::Circuit(Problem &p, std::string n, vec<Variable *> &vars) : AllDifferent(p, n, vars) {
    type = "Circuit";
    set.setCapacity(vars.size(), false);
    //    Kit.control(Stream.of(scp).allMatch(x -> x.dom.areInitValuesExactly(pb.api.range(scp.length))));
}
