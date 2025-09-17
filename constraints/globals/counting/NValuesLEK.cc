//
// Created by audemard on 26/04/2018.
//

#include "NValuesLEK.h"

#include <set>

#include "solver/Solver.h"

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool NValuesLEK::isSatisfiedBy(vec<int> &tuple) { return countDistinct(tuple) <= k; }


bool NValuesLEK::isCorrectlyDefined() { return true; }


int NValuesLEK::countDistinct(vec<int> &tuple) {
    std::set<int> s(tuple.cbegin(), tuple.cend());
    return s.size();
}


//----------------------------------------------
// Filtering
//----------------------------------------------

bool NValuesLEK::filter(Variable *x) {
    if(x->size() == 1) {
        myset.clear();
        for(Variable *y : scope)
            if(y->size() == 1)
                myset.insert(y->value());
        // set.add(map.get(y.dom.firstValue()));
        if(static_cast<int>(myset.size()) > k)
            return false;

        if((int)myset.size() == k) {
            for(Variable *y : scope)
                if(y->size() > 1) {
                    for(int idv : y->domain) {
                        int v = y->domain.toVal(idv);
                        if(myset.find(v) == myset.end() && solver->delVal(y, v) == false)
                            return false;
                    }
                }
            /*                    for(int v : myset) {
                                    if(solver->delVal(y, v) == false)
                                        return false;
             */
            solver->entail(this);
        }
    }
    return true;
}


//----------------------------------------------
// Objective constraint
//----------------------------------------------


// Functions related to Objective constraint
void NValuesLEK::updateBound(long bound) { k = bound; }


long NValuesLEK::maxUpperBound() { return scope.size(); }


long NValuesLEK::minLowerBound() { return 1; }


long NValuesLEK::computeScore(vec<int> &solution) { return countDistinct(solution); }

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------


NValuesLEK::NValuesLEK(Problem &p, std::string n, vec<Variable *> &vars, int kk)
    : GlobalConstraint(p, n, "NValues <= k", vars), k(kk) { }
