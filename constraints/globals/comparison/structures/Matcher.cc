//
// Created by audemard on 26/03/25.
//

#include "Matcher.h"

#include <queue>
#include <stack>

#include "solver/Solver.h"

using namespace Cosoco;

Matcher::Matcher(Constraint* cc)
    : constraint(cc), scope(cc->scope), unfixed(cc->scope.size(), true), fixedVars(cc->scope.size()) {
    arity    = constraint->scope.size();
    minValue = constraint->scope[0]->minimum();
    maxValue = constraint->scope[0]->maximum();
    for(Variable* c : constraint->scope) {
        if(minValue > c->minimum())
            minValue = c->minimum();
        if(maxValue < c->maximum())
            maxValue = c->maximum();
    }
    interval      = maxValue - minValue + 1;
    nNodes        = arity + interval + 1;
    T             = arity + interval;
    nVisitedNodes = 0;
    splitSCC      = false;

    stackTarjan.setCapacity(nNodes, false);
    neighborsOfT.setCapacity(interval, false);
    valsInSCC.setCapacity(interval, false);
    varsOutSCC.setCapacity(arity, false);
    neighborsOfValues.growTo(interval);
    for(auto& tmp : neighborsOfValues) tmp.setCapacity(arity + 1, false);

    var2val = new int[arity];
    val2var = new int[interval];
    std::fill_n(var2val, arity, -1);
    std::fill_n(val2var, interval, -1);

    predBFS = new int[arity];
    std::fill_n(predBFS, arity, -1);
    time      = 1;
    visitTime = new long[nNodes];
    std::fill_n(visitTime, nNodes, -1);
    numDFS = new int[nNodes];
    std::fill_n(numDFS, nNodes, 0);
    lowLink = new int[nNodes];
    std::fill_n(lowLink, nNodes, 0);
}

void Matcher::notifyDeleteDecision(Variable* x, int v, Solver& s) {
    if(unfixed.isLimitRecordedAtLevel(s.decisionLevel() + 1))
        unfixed.restoreLimit(s.decisionLevel() + 1);
    if(fixedVars.isLimitRecordedAtLevel(s.decisionLevel() + 1))
        fixedVars.restoreLimit(s.decisionLevel() + 1);
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
        if(findMatchingFor(idx) == false)
            return false;
    }
    return true;
}


void Matcher::removeInconsistentValues() {
    time++;
    computeNeighbors();
    stackTarjan.clear();
    splitSCC      = false;
    nVisitedNodes = 0;
    /*int nb        = 0;
    for(int idx = 0; idx < arity; idx++) {
        nb += scope[idx]->size();
    }*/
    for(int idx = 0; idx < arity; idx++) {
        if(fixedVars.contains(idx))
            continue;
        if(visitTime[idx] < time)
            tarjanRemoveValues(idx);
    }
    /*int nb2 = 0;
    for(int idx = 0; idx < arity; idx++) {
        nb2 += scope[idx]->size();
    }
    std::cout << nb << " " << nb2 << std::endl;
    */
}


void Matcher::computeNeighbors() {
    int level = constraint->solver->decisionLevel();
    for(SparseSet& set : neighborsOfValues) set.clear();
    neighborsOfT.clear();
    for(int idx = 0; idx < arity; idx++) {
        if(fixedVars.contains(idx))
            continue;
        Variable* x = scope[idx];
        if(x->size() == 1) {   // we discard trivial SCCs (variable assignments) after treating them
            int v = x->value();
            for(int idy : unfixed) {
                if(idy != idx) {
                    if(scope[idy]->containsValue(v)) {
                        constraint->solver->delVal(scope[idy], v);
                    }
                }
            }
            if(fixedVars.isLimitRecordedAtLevel(level) == false)
                fixedVars.recordLimit(level);
            fixedVars.add(idx);
            continue;
        }
        for(int idv : x->domain) {
            int nv = normalizedValue(x->domain.toVal(idv));
            neighborsOfValues[nv].add(idx);
            if(val2var[nv] == idx)
                neighborsOfValues[nv].add(arity);   // E3
            else {
                neighborsOfValues[nv].add(idx);   // E2
                if(val2var[nv] == -1)             // unmatched values
                    neighborsOfT.add(nv);         // E4
            }
        }
    }
}


void Matcher::tarjanRemoveValues(int node) {
    assert(visitTime[node] < time);
    visitTime[node] = time;
    numDFS[node] = lowLink[node] = ++nVisitedNodes;
    stackTarjan.add(node);

    if(node < arity) {   // node for a variable
        int adjacentNode = arity + var2val[node];
        if(visitTime[adjacentNode] != time) {   // This code is repeated 3 times to save stacking (recursive calls)
            tarjanRemoveValues(adjacentNode);
            lowLink[node] = std::min(lowLink[node], lowLink[adjacentNode]);
        } else if(stackTarjan.contains(adjacentNode))
            lowLink[node] = std::min(lowLink[node], numDFS[adjacentNode]);
    } else if(node < T) {   // node for a value
        SparseSet& neighbors = neighborsOfValues[node - arity];
        for(int i = 0; i < neighbors.size(); i++) {
            int adjacentNode = neighbors[i] == arity ? T : neighbors[i];
            if(visitTime[adjacentNode] != time) {
                tarjanRemoveValues(adjacentNode);
                lowLink[node] = std::min(lowLink[node], lowLink[adjacentNode]);
            } else if(stackTarjan.contains(adjacentNode))
                lowLink[node] = std::min(lowLink[node], numDFS[adjacentNode]);
        }
    } else {
        assert(node == T);   // node for T
        for(int i = 0; i < neighborsOfT.size(); i++) {
            int adjacentNode = arity + neighborsOfT[i];
            if(visitTime[adjacentNode] != time) {
                tarjanRemoveValues(adjacentNode);
                lowLink[node] = std::min(lowLink[node], lowLink[adjacentNode]);
            } else if(stackTarjan.contains(adjacentNode))
                lowLink[node] = std::min(lowLink[node], numDFS[adjacentNode]);
        }
    }
    if(lowLink[node] == numDFS[node]) {   // if node is the root of a SCC
        splitSCC = splitSCC || lowLink[node] > 1 || nVisitedNodes < nNodes;
        if(splitSCC) {
            // first, we compute varsOutSCC and valsInSCC
            varsOutSCC.resetTo(unfixed);
            valsInSCC.clear();
            int nodeSCC = -1;
            // assert(stackTarjan.contains(node));
            // stackTarjan.display();
            // std::cout << "\n";
            while(nodeSCC != node) {
                // std::cout << nodeSCC << " " << node << std::endl;
                assert(stackTarjan.size() > 0);
                nodeSCC = stackTarjan.pop();
                if(nodeSCC < arity)
                    varsOutSCC.del(nodeSCC);
                else if(nodeSCC < T)
                    valsInSCC.add(nodeSCC - arity);
            }

            // second, we remove appropriate values (linking values in the CSS with variables outside the SCC)
            if(varsOutSCC.size() > 0 && valsInSCC.size() > 0) {
                for(int nv : valsInSCC) {
                    int        v         = domainValue(nv);
                    SparseSet& neighbors = neighborsOfValues[nv];
                    if(neighbors.size() < varsOutSCC.size()) {
                        for(int idx : neighbors) {
                            if(idx < arity && varsOutSCC.contains(idx) && var2val[idx] != nv)
                                constraint->solver->delVal(scope[idx], v);
                        }
                    } else
                        for(int idx : varsOutSCC) {
                            if(scope[idx]->containsValue(v) && var2val[idx] != nv)
                                constraint->solver->delVal(scope[idx], v);
                        }
                }
            }
        }
    }
}