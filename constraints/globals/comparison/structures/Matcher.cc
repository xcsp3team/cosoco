//
// Created by audemard on 26/03/25.
//

#include "Matcher.h"

#include <queue>
#include <stack>

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
    visitTime = new long[arity + interval + 1];
    std::fill_n(visitTime, arity + interval + 1, -1);
}

void Matcher::notifyDeleteDecision(Variable* x, int v, Solver& s) {
    if(unfixed.isLimitRecordedAtLevel(s.decisionLevel() + 1))
        unfixed.restoreLimit(s.decisionLevel() + 1);
}


bool Matcher::findMatchingFor(int x) {
    std::stack<int> stack;
    assert(stack.size() == 0);
    time++;
    predBFS[x] = -1;
    stack.push(x);
    while(!stack.empty()) {
        int y = stack.top();
        stack.pop();
        // std::cout << "y=" << y << std::endl;
        for(int a : scope[y]->domain) {
            int nv = normalizedValue(scope[y]->domain.toVal(a));
            int z  = val2var[nv];
            // std::cout << "nv=" << nv << " z=" << z << " " << var2val[z] << std::endl;
            assert(z == -1 || var2val[z] == nv);
            if(z == -1) {   // we have found a free value, so we are good
                while(predBFS[y] != -1) {
                    assert(predBFS[y] != y);
                    int nw      = var2val[y];
                    var2val[y]  = nv;
                    val2var[nv] = y;
                    nv          = nw;
                    y           = predBFS[y];
                }
                var2val[y]  = nv;
                val2var[nv] = y;
                return true;
            } else if(visitTime[z] < time) {
                // std::cout << "visit " << z << " " << y << "\n";
                visitTime[z] = time;
                predBFS[z]   = y;
                stack.push(z);
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
            unmatched.push(idx);
        else {
            assert(val2var[nv] == idx);
            if(constraint->scope[idx]->containsValue(domainValue(nv)) == false) {
                var2val[idx] = val2var[nv] = -1;
                unmatched.push(idx);
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
        unmatched.pop();
        //  std::cout << "idx" << idx << std::endl;
        if(findMatchingFor(idx) == false)
            return false;
    }
    return true;
}