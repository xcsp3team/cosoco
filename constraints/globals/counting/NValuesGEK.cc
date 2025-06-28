//
// Created by audemard on 26/04/2018.
//

#include "NValuesGEK.h"

#include <set>

#include "solver/Solver.h"

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool NValuesGEK::isSatisfiedBy(vec<int> &tuple) { return countDistinct(tuple) >= k; }


bool NValuesGEK::isCorrectlyDefined() { return true; }


unsigned int NValuesGEK::countDistinct(vec<int> &tuple) {
    std::set<int> s(tuple.cbegin(), tuple.cend());
    return s.size();
}


//----------------------------------------------
// Filtering
//----------------------------------------------


void NValuesGEK::initializeSets() {
    fixedValues.clear();
    unfixedVariables.clear();

    for(int posx = 0; posx < scope.size(); posx++) {
        if(scope[posx]->size() == 1)
            fixedValues.insert(scope[posx]->value());
        else
            unfixedVariables.insert(posx);
    }

    /*    for(int i = unfixedVariables.limit; i >= 0; i--) {
            int     x   = unfixedVariables.dense[i];
            Domain &dom = list[x].dom;

            if(dom.size() > fixedVals.size())
                continue;

            int sentinel = sentinels[x];
            if(dom.containsValue(sentinel) && !fixedVals.count(sentinel))
                continue;

            if(dom.size() > 5)   // hardcoded threshold
                continue;

            bool found = false;
            for(int a = dom.first(); a != -1; a = dom.next(a)) {
                int v = dom.toVal(a);
                if(!fixedVals.count(v)) {
                    sentinels[x] = v;
                    found        = true;
                    break;
                }
            }

            if(!found)
                relevantUnfixedVarsSet.removeAtPosition(i);   // all domain values are in fixedVals
        }
    */
}
bool NValuesGEK::filter(Variable *x) {
    if(x->size() == 1) {
        initializeSets();
        if(fixedValues.size() + unfixedVariables.size() < k)
            return false;
        if(fixedValues.size() + unfixedVariables.size() == k) {
            for(int posx : unfixedVariables) {
                Variable *x = scope[posx];
                for(int v : fixedValues)
                    if(solver->delVal(x, v) == false)
                        return false;
            }
            if(unfixedVariables.size() == 0)
                solver->entail(this);
        }
    }
    return true;
}


//----------------------------------------------
// Objective constraint
//----------------------------------------------


// Functions related to Objective constraint
void NValuesGEK::updateBound(long bound) { k = bound; }


long NValuesGEK::maxUpperBound() { return scope.size(); }


long NValuesGEK::minLowerBound() { return 1; }


long NValuesGEK::computeScore(vec<int> &solution) { return countDistinct(solution); }

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------


NValuesGEK::NValuesGEK(Problem &p, std::string n, vec<Variable *> &vars, int kk)
    : GlobalConstraint(p, n, "NValues >= k", vars), k(kk) { }
