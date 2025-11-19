//
// Created by audemard on 19/11/2025.
//

#include "MaximumArg.h"

#include "Solver.h"

using namespace Cosoco;
//----------------------------------------------
// Check validity
//----------------------------------------------

bool MaximumArg::isSatisfiedBy(vec<int> &tuple) {
    int v = tuple[tuple[0] + 1];
    for(int i = 1; i < tuple.size(); i++)
        if(tuple[i] > v)
            return false;
    if(rank == XCSP3Core::FIRST)
        for(int i = 1; i <= tuple[0]; i++)
            if(tuple[i] == v)
                return false;
    if(rank == XCSP3Core::LAST)
        for(int i = tuple[0] + 2; i < tuple.size(); i++)
            if(tuple[i] == v)
                return false;
    return true;
}

//----------------------------------------------
// Filtering
//----------------------------------------------

bool MaximumArg::filter(Variable *dummy) {
    // we compute maxMin and maxMax
    int maxMin = INT_MIN, maxMax = INT_MIN;
    for(int idv : index->domain) {
        maxMin = std::max(maxMin, list[idv]->minimum());
        maxMax = std::max(maxMax, list[idv]->maximum());
    }

    // we remove too large values in other variables (those that cannot be anymore used to be indexed)
    int maxMind = INT_MIN, maxMaxd = INT_MIN;   // d for deleted indexes
    for(int a = index->domain.lastRemoved(); a != -1; a = index->domain.prevRemoved(a)) {
        if(solver->delValuesGreaterOrEqualThan(list[a], maxMax + 1) == false)
            return false;
        maxMind = std::max(maxMind, list[a]->minimum());
        maxMaxd = std::max(maxMaxd, list[a]->maximum());
    }

    // we remove some values from the domain of the index variable
    int limit = std::max(maxMin, maxMind);
    for(int idv : index->domain)
        if(list[idv]->maximum() < limit)
            solver->delIdv(index, idv);   // no inconsistency possible

    if(rank == XCSP3Core::FIRST) {
        bool safe = false, sing = false;
        for(int idx = 0; idx < list.size(); idx++) {
            if(list[idx]->containsValue(maxMax)) {
                if(index->containsIdv(idx) == false) {
                    if(safe == false && solver->delVal(list[idx], maxMax) == false)
                        return false;
                } else {
                    safe = true;
                    if(sing) {
                        solver->delIdv(index, idx);   // no inconsistency possible
                        maxMind = std::max(maxMind, list[idx]->minimum());
                        maxMaxd = std::max(maxMaxd, list[idx]->maximum());
                    } else if(list[idx]->size() == 1)
                        sing = true;
                }
            }
        }
    }


    if(rank == XCSP3Core::LAST) {
        bool safe = false, sing = false;
        for(int idx = list.size() - 1; idx >= 0; idx--)
            if(list[idx]->containsValue(maxMax)) {
                if(index->containsIdv(idx) == false) {
                    if(!safe && solver->delVal(list[idx], maxMax) == false)
                        return false;
                } else {
                    safe = true;
                    if(sing) {
                        solver->delIdv(index, idx);   // no inconsistency possible
                        maxMind = std::max(maxMind, list[idx]->minimum());
                        maxMaxd = std::max(maxMaxd, list[idx]->maximum());
                    } else if(list[idx]->size() == 1)
                        sing = true;
                }
            }
    }

    if(index->size() == 1) {
        int a = index->value();
        if(solver->delValuesLowerOrEqualThan(list[a], maxMind - 1) == false)
            return false;
        if(rank == XCSP3Core::ANY && list[a]->minimum() >= maxMaxd)
            // if (list[a].dom.firstValue() >= maxMaxd - (rank == TypeRank.ANY ? 0 : 1)) // Not correct
            return solver->entail(this);
    }
    return true;
}


//----------------------------------------------
// Constructor and initialisation methods
//----------------------------------------------
MaximumArg::MaximumArg(Problem &p, std::string n, vec<Variable *> &vars, Variable *v, XCSP3Core::RankType r)
    : GlobalConstraint(p, n, "Maximum Arg", Constraint::createScopeVec(v, &vars)), rank(r) {
    index = v;
    vars.copyTo(list);
}
