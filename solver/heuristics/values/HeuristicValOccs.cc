//
// Created by audemard on 16/11/22.
//

#include "HeuristicValOccs.h"

#include "solver/Solver.h"

using namespace Cosoco;


HeuristicValOccs::HeuristicValOccs(Solver &s) : HeuristicVal(s) {
    for(int i = 0; i < solver.problem.variablesArray.size(); i++) {
        nbOccurrences.push();
        nbOccurrences.last().growTo(solver.problem.variablesArray[i][0]->size());   // Deal with identical domains
        lastConflict.push(-1);
    }
}

void HeuristicValOccs::updateOccurrences(int array) {
    nbOccurrences[array].fill(0);
    for(Variable *x : solver.problem.variablesArray[array]) {
        if(x->size() > 1)
            continue;
        nbOccurrences[array][x->valueId()]++;
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

    int tmp = -1;
    for(int idv = 0; idv < nbOccurrences[x->array].size(); idv++) {
        if(x->containsIdv(idv) && (tmp == -1 || nbOccurrences[x->array][tmp] > nbOccurrences[x->array][idv]))
            tmp = idv;
    }
    assert(tmp >= 0);
    return tmp;
}
