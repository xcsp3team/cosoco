//
// Created by audemard on 26/03/25.
//

#include "Matcher.h"

#include <queue>
#include <stack>

#include "solver/Solver.h"

using namespace Cosoco;

MatcherAllDifferent::MatcherAllDifferent(Constraint* c) : Matcher(c) {
    val2var = new int[interval];
    std::fill_n(val2var, interval, -1);
    predBFS = new int[arity];
    std::fill_n(predBFS, arity, -1);
}

MatcherCardinality::MatcherCardinality(Constraint* c, vec<int>& _keys, vec<int>& _minOccs, vec<int>& _maxOccs) : Matcher(c) {
    _keys.copyTo(keys);

    predBFS = new int[std::max(arity, interval)];
    std::fill_n(predBFS, std::max(arity, interval), -1);
    queueBFS.setCapacity(std::max(interval, arity), false);

    minOccs = new int[interval];
    maxOccs = new int[interval];
    std::fill_n(maxOccs, interval, INT_MAX);
    for(int i = 0; i < keys.size(); i++) {
        minOccs[normalizedValue(keys[i])] = _minOccs[i];
        maxOccs[normalizedValue(keys[i])] = _maxOccs[i];
    }

    predValue = new int[interval];
    std::fill_n(predValue, interval, -1);

    possibleVars.growTo(interval);
    for(int u = 0; u < interval; u++) {
        possibleVars[u].setCapacity(arity, false);
        for(int posx = 0; posx < arity; posx++)
            if(scope[posx]->containsValue(domainValue(u)))
                possibleVars[u].add(posx);
    }

    val2var.growTo(interval);
    for(int i = 0; i < interval; i++) val2var[i].setCapacity(arity, false);
}


Matcher::Matcher(Constraint* cc)
    : constraint(cc), scope(cc->scope), unfixed(cc->scope.size(), true), fixedVars(cc->scope.size()) {
    arity    = constraint->scope.size();
    minValue = constraint->scope[0]->minimum();
    maxValue = constraint->scope[0]->maximum();
    for(Variable* x : constraint->scope) {
        if(minValue > x->minimum())
            minValue = x->minimum();
        if(maxValue < x->maximum())
            maxValue = x->maximum();
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
    std::fill_n(var2val, arity, -1);


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


bool MatcherAllDifferent::findMatchingFor(int x) {
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

bool MatcherAllDifferent::findMaximumMatching() {
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


void MatcherAllDifferent::computeNeighbors() {
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


void MatcherCardinality::handleAugmentingPath(int x, int v) {
    while(predBFS[v] != -1) {
        int y      = predBFS[v];
        var2val[x] = v;
        val2var[v].add(x);
        val2var[v].del(y);
        x = y;
        v = predValue[v];
    }
    var2val[x] = v;
    val2var[v].add(x);
}

bool MatcherCardinality::findMatchingForValue(int u) {
    time++;
    queueBFS.clear();
    queueBFS.add(u);
    predBFS[u]   = -1;
    visitTime[u] = time;
    while(!queueBFS.isEmpty()) {
        int v = queueBFS.shift();
        for(int posx : possibleVars[v]) {
            if(scope[posx]->containsValue(domainValue(v))) {
                int w = var2val[posx];
                if(w == -1) {
                    handleAugmentingPath(posx, v);
                    return true;
                } else if(w != v) {
                    if(val2var[w].size() > minOccs[w] && var2val[posx] == w) {
                        val2var[w].del(posx);
                        handleAugmentingPath(posx, v);
                        return true;
                    } else if(visitTime[w] < time) {
                        visitTime[w] = time;
                        queueBFS.add(w);
                        predBFS[w]   = posx;
                        predValue[w] = v;
                    }
                }
            }
        }
    }
    return false;
}

bool MatcherCardinality::findMatchingForVariable(int x) {
    time++;
    queueBFS.clear();
    queueBFS.add(x);

    predBFS[x]   = -1;
    visitTime[x] = time;
    while(!queueBFS.isEmpty()) {
        int y = queueBFS.shift();

        for(int idv : scope[y]->domain) {
            int u = normalizedValue(scope[y]->domain.toVal(idv));
            if(val2var[u].size() < maxOccs[u]) {
                while(predBFS[y] != -1) {
                    int v      = var2val[y];   // previous value
                    var2val[y] = u;
                    val2var[u].add(y);
                    val2var[v].del(y);
                    y = predBFS[y];
                    u = v;
                }
                var2val[y] = u;
                val2var[u].add(y);
                return true;
            }
            for(int z : val2var[u]) {
                assert(var2val[z] == u);
                if(visitTime[z] < time) {
                    visitTime[z] = time;
                    predBFS[z]   = y;
                    queueBFS.add(z);
                }
            }
        }
    }
    return false;
}

void MatcherCardinality::computeNeighbors() {
    for(auto& set : neighborsOfValues) set.clear();
    for(int u = 0; u < interval; u++) {
        if(val2var[u].size() < maxOccs[u])
            neighborsOfT.add(u);
        else
            neighborsOfT.del(u);
        if(val2var[u].size() > minOccs[u])
            neighborsOfValues[u].add(arity);
        else
            neighborsOfValues[u].del(arity);
        for(int posx : possibleVars[u]) {
            if(scope[posx]->containsValue(domainValue(u)) && var2val[posx] != u)
                neighborsOfValues[u].add(posx);
            else
                neighborsOfValues[u].del(posx);
        }
    }
}

bool MatcherCardinality::findMaximumMatching() {
    // Make sure each variable is not matched with a value that is not in its domain anymore
    for(int posx = 0; posx < arity; posx++) {
        int u = var2val[posx];
        if(u == -1 || scope[posx]->containsValue(domainValue(u)) == false) {
            if(scope[posx]->size() == 1) {
                int v = normalizedValue(scope[posx]->value());
                if(u != -1)
                    val2var[u].del(posx);
                if(maxOccs[v] == val2var[v].size()) {
                    var2val[posx] = -1;
                } else {
                    var2val[posx] = v;
                    val2var[v].add(posx);
                }
            } else if(u != -1) {
                val2var[u].del(posx);
                var2val[posx] = -1;
            }
        }
    }


    // Generate a feasible flow (part of the matching)
    for(int i = 0; i < keys.size(); i++) {
        int u = normalizedValue(keys[i]);
        while(val2var[u].size() < minOccs[u])
            if(findMatchingForValue(u) == false) {
                return false;
            }
    }
    /*    for(int i = 0; i < arity; i++) std::cout << var2val[i] << " ";
        std::cout << std::endl;

        for(auto& s : val2var) {
            s.display();
            std::cout << "\n";
        }
    */

    int level = constraint->solver->decisionLevel();
    unmatched.clear();
    for(int x = 0; x < arity; x++) {
        if(var2val[x] == -1)
            unmatched.push(x);
        else if(scope[x]->size() == 1 && unfixed.contains(x)) {
            if(unfixed.isLimitRecordedAtLevel(level) == false)
                unfixed.recordLimit(level);
            unfixed.del(x);
        }
    }
    while(unmatched.size() > 0) {
        int idx = unmatched.last();
        unmatched.pop();
        if(findMatchingForVariable(idx) == false) {
            //      std::cout << "BBB\n";
            return false;
        }
    }
    return true;
}