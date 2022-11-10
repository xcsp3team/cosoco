#include "ExactlyK.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool ExactlyK::isSatisfiedBy(vec<int> &tuple) {
    int nb = 0;
    for(int v : tuple)
        if(v == value)
            nb++;
    return nb == k;
}


bool ExactlyK::isCorrectlyDefined() {
    if(k < 0 || k > scope.size())
        throw std::logic_error("Constraint " + std::to_string(idc) + ": ExactlyK must have 0 <= k <= size(list)");
    return true;
}

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool ExactlyK::filter(Variable *dummy) {
    int nbGuaranteedOccurrences = 0, nbPossibleOccurrences = 0;

    if(k == 0) {
        for(int posx : unassignedVariablesIdx)
            if(solver->delVal(scope[posx], value) == false)
                return false;
        done = true;
        return true;
    }

    int i = 0;
    for(Variable *x : scope) {
        if(valueToidv[i] != -1 && x->containsIdv(valueToidv[i])) {
            nbPossibleOccurrences++;
            if(x->size() == 1 && ++nbGuaranteedOccurrences > k)
                return false;
        }
        i++;
    }

    if(nbGuaranteedOccurrences == k) {
        int toremove = nbPossibleOccurrences - k;
        for(int posx : unassignedVariablesIdx) {
            Variable *x = scope[posx];
            if(x->size() > 1 && valueToidv[posx] != -1 && x->containsIdv(valueToidv[posx])) {
                solver->delIdv(x, valueToidv[posx]);   // delvalue can not be false ?
                if(--toremove == 0)
                    break;
            }
        }
        solver->entail(this);
        return true;
    }

    if(nbPossibleOccurrences < k)
        return false;
    if(nbPossibleOccurrences == k) {
        int toassign = k - nbGuaranteedOccurrences;
        for(int posx : unassignedVariablesIdx) {
            Variable *x = scope[posx];
            if(x->size() > 1 && valueToidv[posx] != -1 && x->containsIdv(valueToidv[posx])) {
                if(solver->assignToIdv(x, valueToidv[posx]) == false)
                    return false;
                toassign--;
                if(toassign == 0)
                    break;
            }
        }
        solver->entail(this);
    }
    return true;
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

ExactlyK::ExactlyK(Problem &p, std::string n, vec<Variable *> &vars, int kk, int val)
    : GlobalConstraint(p, n, "Exactly K", vars), k(kk), value(val), done(false) {
    valueToidv.growTo(scope.size(), -1);
    for(int i = 0; i < scope.size(); i++) {
        if(scope[i]->containsValue(value))
            valueToidv[i] = scope[i]->domain.toIdv(value);
    }
}


State ExactlyK::status() { return done ? CONSISTENT : UNDEF; }


void ExactlyK::reinitialize() { done = false; }
