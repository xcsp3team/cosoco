//
// Created by audemard on 16/11/22.
//

#include "solver/heuristics/values/HeuristicValOccs.h"

#include "mtl/Map.h"
#include "solver/Solver.h"
using namespace Cosoco;


HeuristicValOccs::HeuristicValOccs(Solver &s) : HeuristicVal(s) {
    for(int i = 0; i < solver.problem.variablesArray.size(); i++) {
        nbOccurrences.push();
        elements.push();
        // Put all elements in Map.
        for(Variable *x : solver.problem.variablesArray[i]) {
            for(int idv : x->domain) {
                int v = x->domain.toVal(idv);
                if(nbOccurrences.last().has(v) == false) {
                    nbOccurrences.last().insert(v, 0);
                    elements.last().push(v);
                }
            }
        }
        lastConflict.push(-1);
    }
}

void HeuristicValOccs::updateOccurrences(int array) {
    // init
    for(int v : elements[array]) nbOccurrences[array][v] = 0;

    for(Variable *x : solver.problem.variablesArray[array]) {
        if(x->size() > 1)
            continue;
        nbOccurrences[array][x->value()]++;
    }
}

int HeuristicValOccs::select(Variable *x) {
    // Special case
    if(x->size() == 1)
        return x->domain.firstId();
    if(x->array == -1)
        return x->domain.firstId();

    if(solver.conflicts != lastConflict[x->array]) {
        updateOccurrences(x->array);
        lastConflict[x->array] = solver.conflicts;
    }
    bool ok = false;
    int  v  = -1;
    for(int v2 : elements[x->array]) {
        if(x->containsValue(v2) && (ok == false || nbOccurrences[x->array][v] >= nbOccurrences[x->array][v2])) {
            ok = true;
            v  = v2;
        }
    }
    return x->domain.toIdv(v);
}
