//
// Created by audemard on 26/03/25.
//

#include "Matcher.h"

#include <queue>

#include "solver/Solver.h"

using namespace Cosoco;

Matcher::Matcher(Constraint* cc) : constraint(cc), scope(cc->scope), unfixed(cc->scope.size()) {
    arity    = constraint->scope.size();
    minValue = constraint->scope[0]->minimum();
    maxValue = constraint->scope[0]->maximum();
    for(Variable* c : constraint->scope) {
        if(minValue > c->minimum())
            minValue = c->minimum();
        if(maxValue < c->maximum())
            maxValue = c->maximum();
    }
    interval = maxValue - minValue + 1;
    var2val  = new int[arity];
    val2var  = new int[interval];
    std::fill_n(var2val, arity, -1);
    std::fill_n(val2var, interval, -1);
    predBFS = new int[arity];
    std::fill_n(predBFS, arity, -1);
    time      = 1;
    visitTime = new unsigned long[arity + interval + 1];
    std::fill_n(visitTime, arity + interval + 1, 0);
    unmatched.capacity(arity);
}

void Matcher::notifyDeleteDecision(Variable* x, int v, Solver& s) {
    if(unfixed.isLimitRecordedAtLevel(s.decisionLevel() + 1))
        unfixed.restoreLimit(s.decisionLevel() + 1);
}


bool Matcher::findMatchingFor(int idx) {
    time++;
    std::queue<int> queue;
    queue.push(idx);
    predBFS[idx] = -1;
    while(queue.empty() == false) {
        int idy = queueBFS.front();
        queue.pop();
        std::cout << "ici" << idy << std::endl;
        for(int idvy : scope[idy]->domain) {
            int nv  = normalizedValue(scope[idy]->domain.toVal(idvy));
            int idz = val2var[nv];
            assert(idz == -1 || var2val[idz] == nv);
            if(idz == -1) {   // we have found a free value, so we are good
                while(predBFS[idy] != -1) {
                    assert(predBFS[idy] != idy);
                    std::cout << predBFS[idy] << " " << idy << " " << predBFS[idy] << std::endl;
                    int nw       = var2val[idy];
                    var2val[idy] = nv;
                    val2var[nv]  = idy;
                    nv           = nw;
                    idy          = predBFS[idy];
                }
                var2val[idy] = nv;
                val2var[nv]  = idy;
                return true;
            }
            if(visitTime[idz] < time) {
                std::cout << time << " " << idz << " " << visitTime[idz] << "  " << idy << std::endl;

                visitTime[idz] = time;
                predBFS[idz]   = idy;
                queue.push(idz);
            }
        }
    }
    return false;
}

bool Matcher::findMaximumMatching() {
    unmatched.clear();
    int level = constraint->solver->decisionLevel();
    for(int idx = 0; idx < arity; idx++) {   // Find unmatched variables
        int nv = var2val[idx];
        if(nv == -1)
            unmatched.push_(idx);
        else {
            assert(val2var[nv] == idx);
            if(constraint->scope[idx]->containsValue(domainValue(nv)) == false) {
                var2val[idx] = val2var[nv] = -1;
                unmatched.push_(idx);
            }
            if(constraint->scope[idx]->size() == 1 && unfixed.contains(idx)) {
                if(unfixed.isLimitRecordedAtLevel(level) == false)
                    unfixed.recordLimit(level);
                unfixed.del(idx);
            }
        }
    }

    while(unmatched.size() > 0) {   // Find maximum matching
        int idx = unmatched.last();
        unmatched.pop_();
        if(findMatchingFor(idx) == false)
            return false;
    }
    return true;
}