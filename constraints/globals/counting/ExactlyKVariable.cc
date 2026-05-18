#include "ExactlyKVariable.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool ExactlyKVariable::isSatisfiedBy(vec<int> &tuple) {
    return positionOfKInList != -1 ? tuple.countOccurrencesOf(value) == tuple[positionOfKInList]
                                   : tuple.countOccurrencesOf(value, 0, tuple.size() - 1) == tuple.last();
}


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool ExactlyKVariable::filter(Cosoco::Variable *dummy) {
    // counting the nb of occurrences of value in the vector
    int nbGuaranteedOccurrences = 0, nbPossibleOccurrences = 0;
    for(Variable *x : list)
        if(x->containsValue(value)) {
            nbPossibleOccurrences++;
            if(x->size() == 1)
                nbGuaranteedOccurrences++;
        }

    if(k->size() == 1) {
        int valK = k->value();
        if(valK < nbGuaranteedOccurrences || nbPossibleOccurrences < valK)
            return false;
    } else {
        // possible update of the domain of k when present in the vector, first by removing value (if present)
        // so as to update immediately nbPossibleOccurrences
        if(positionOfKInList != -1) {
            if(k->containsValue(value)) {
                bool deleted = false;
                for(int idk : reverse(k->domain)) {   // Reverse traversal because of deletion
                    int v = k->domain.toVal(idk);
                    if(v == value) {
                        if(value < nbGuaranteedOccurrences + 1 ||
                           nbPossibleOccurrences < value) {   // +1 by assuming we assign the value
                            if(solver->delVal(k, value) == false)
                                return false;
                            deleted = true;
                        }
                    } else {
                        if(v < nbGuaranteedOccurrences ||
                           nbPossibleOccurrences - 1 < v) {   // -1 by assuming we assign vx (and not value)
                            if(solver->delVal(k, v) == false)
                                return false;
                        }
                    }
                }
                if(deleted)
                    nbPossibleOccurrences--;
            } else {
                if(solver->delValuesLowerOrEqualThan(k, nbGuaranteedOccurrences - 1) == false ||
                   solver->delValuesGreaterOrEqualThan(k, nbPossibleOccurrences + 1) == false)
                    return false;
            }
        } else if(solver->delValuesLowerOrEqualThan(k, nbGuaranteedOccurrences - 1) == false ||
                  solver->delValuesGreaterOrEqualThan(k, nbPossibleOccurrences + 1) == false)
            return false;
    }

    // if k is singleton, updating the domain of the other variables
    if(k->size() == 1) {
        int valK = k->value();
        if(valK == nbGuaranteedOccurrences) {
            for(Variable *x : list)
                if(x->size() > 1 && x->containsValue(value) && solver->delVal(x, value) == false)
                    return false;
            solver->entail(this);
        }
        if(valK == nbPossibleOccurrences) {
            for(Variable *x : list)
                if(x->size() > 1)
                    solver->assignToVal(x, value);
            solver->entail(this);
        }
    }
    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

ExactlyKVariable::ExactlyKVariable(Problem &p, std::string n, vec<Variable *> &vars, Variable *kk, int val)
    : GlobalConstraint(p, n, "Exactly K variable", Constraint::createScopeVec(&vars, kk)), k(kk), value(val) {
    positionOfKInList = vars.firstOccurrenceOf(k);
    vars.copyTo(list);
}
