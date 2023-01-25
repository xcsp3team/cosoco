#include "ElementConstant.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity
//----------------------------------------------

bool ElementConstant::isSatisfiedBy(vec<int> &tuple) {
    int idx = indexInList == -1 ? tuple[tuple.size() - 1] : tuple[indexInList];
    return tuple[idx - (startAtOne ? 1 : 0)] == result;
}


//----------------------------------------------
// Filtering
//----------------------------------------------


bool ElementConstant::filter(Variable *dummy) {
    if(indexInList != -1 && solver->decisionLevel() == 0 && indexInList + (startAtOne ? 1 : 0) != result) {
        if(solver->delVal(index, result) == false)
            return false;
    }

    if(index->size() > 1) {   // checking that the values of index are still valid
        for(int idv : reverse(index->domain)) {
            int v = index->domain.toVal(idv);
            if(getVariableFor(v)->containsValue(result) == false && solver->delVal(index, v) == false)
                return false;
        }
    }

    // Not else : domain can have changed
    if(index->size() == 1) {
        Variable *x = getVariableFor(index->value());   // This value must be equal to result
        if(solver->assignToVal(x, result) == false)
            return false;
        solver->entail(this);
    }
    return true;
}


//----------------------------------------------
// Constructor and initialisation methods
//----------------------------------------------


ElementConstant::ElementConstant(Problem &p, std::string n, vec<Variable *> &vars, Variable *i, int kk, bool one)
    : Element(p, n, "Element Constant", Constraint::createScopeVec(&vars, i), i, one), result(kk) {
    szVector    = vars.size();
    indexInList = vars.firstOccurrenceOf(i);
}
