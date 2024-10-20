//
// Created by audemard on 2020-03-17.
//

#include "NValuesEQVar.h"

#include <set>

#include "solver/Solver.h"

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool NValuesEQVar::isSatisfiedBy(vec<int> &tuple) { return countDistinct(tuple) == tuple.last(); }


bool NValuesEQVar::isCorrectlyDefined() { return true; }


int NValuesEQVar::countDistinct(vec<int> &tuple) {
    std::set<int> s(tuple.cbegin(), tuple.cend() - 1);
    return s.size();
}


//----------------------------------------------
// Filtering
//----------------------------------------------


void NValuesEQVar::initializeSets() {
    fixedValues.clear();
    unfixedVariables.clear();
    for(int i = 0; i < scope.size() - 1; i++)
        if(scope[i]->size() == 1)
            fixedValues.insert(scope[i]->value());
        else
            unfixedVariables.insert(i);

    /*
     * bool test = true;

    default boolean iterateOnValuesStoppingWhen(Predicate<Integer> p) {
            for (int a = first(); a != -1; a = next(a))
                if (p.test(toVal(a)))
                    return true;
            return false;
        }


    if(test) {
        for(int pos : unfixedVariables) {
            Variable *x = scope[pos];
            if(x->size() > fixedValues.size())
                continue;
            if(x->size() > 4) // hard coding for avoiding iterating systematically over all values
                continue;
            if(dom.iterateOnValuesStoppingWhen(v-> !fixedVals.contains(v)) == false)
            unfixedVars.removeAtPosition(i);
        }
    }*/
}


bool NValuesEQVar::filter(Variable *x) {
    if(x->size() == 1) {
        initializeSets();
        if(solver->delValuesLowerOrEqualThan(k, fixedValues.size() - 1) == false)
            return false;
        if(solver->delValuesGreaterOrEqualThan(k, fixedValues.size() + unfixedVariables.size() + 1) == false)
            return false;

        //        if(k.dom.removeValues(LT, fixedVals.size()) == false || k.dom.removeValues(GT, fixedVals.size() +
        //        unfixedVars.size()) == false)
        //            return false;
        if(k->size() == 1) {
            unsigned int limit = k->value();
            if(fixedValues.size() == limit) {
                for(int pos : unfixedVariables) {
                    Variable *y = scope[pos];
                    for(int idv : y->domain) {
                        int vy = y->domain.toVal(idv);
                        if(fixedValues.find(vy) == fixedValues.end() && solver->delVal(y, vy) == false)
                            return false;
                    }
                }
                solver->entail(this);
            } else if(fixedValues.size() + unfixedVariables.size() == limit) {
                for(int pos : unfixedVariables) {
                    Variable *y = scope[pos];
                    for(int idv : y->domain) {
                        int vy = y->domain.toVal(idv);
                        if(fixedValues.find(vy) != fixedValues.end() && solver->delVal(y, vy) == false)
                            return false;
                    }
                }
            }
        }
    }

    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

NValuesEQVar::NValuesEQVar(Problem &p, std::string n, vec<Variable *> &vars, Variable *x)
    : GlobalConstraint(p, n, "NValues = x", Constraint::createScopeVec(&vars, x)), k(x) {
    szVector = vars.size();
    assert(vars.firstOccurrenceOf(x) == -1);
}
