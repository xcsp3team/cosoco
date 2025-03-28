//
// Created by audemard on 26/03/25.
//

#include "Matcher.h"

#include <queue>

#include "solver/Solver.h"

using namespace Cosoco;
#define NIL -1
#define INF INT_MAX


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
    pairU    = new int[arity];
    pairV    = new int[interval];
    dist     = new int[arity];

    std::fill_n(pairU, arity, NIL);
    std::fill_n(pairV, interval, NIL);
}

void Matcher::notifyDeleteDecision(Variable* x, int v, Solver& s) {
    if(unfixed.isLimitRecordedAtLevel(s.decisionLevel() + 1))
        unfixed.restoreLimit(s.decisionLevel() + 1);
}


bool Matcher::dfs(int idx) {
    if(idx != NIL) {
        for(int idv : scope[idx]->domain) {
            // Adjacent to u
            int nv = normalizedValue(scope[idx]->domain.toVal(idv));
            assert(nv >= 0 && nv < interval);
            // Follow the distances set by BFS
            if(dist[pairV[nv]] == dist[idx] + 1) {
                if(dfs(pairV[nv])) {
                    pairV[nv]  = idx;
                    pairU[idx] = nv;
                    return true;
                }
            }
        }

        // If there is no augmenting path beginning with u.
        dist[idx] = INF;
        return false;
    }
    return true;
}


bool Matcher::bfs() {
    std::queue<int> Q;   // an integer queue

    // First layer of vertices (set distance as 0)
    for(int idx = 0; idx < arity; idx++) {
        // If this is a free vertex, add it to queue
        if(pairU[idx] == NIL) {
            dist[idx] = 0;
            Q.push(idx);
        } else   // set distance as infinite so that this vertex
            dist[idx] = INF;
    }


    dist[NIL] = INF;   // Initialize distance to NIL as infinite

    while(!Q.empty()) {   // Q is going to contain vertices of left side only.
        // Dequeue a vertex
        int idx = Q.front();
        Q.pop();

        // If this node is not NIL and can provide a shorter path to NIL
        if(dist[idx] < dist[NIL]) {
            // Get all adjacent vertices of the dequeued vertex u
            for(int idv : scope[idx]->domain) {
                int nv = normalizedValue(scope[idx]->domain.toVal(idv));
                assert(nv >= 0 && nv < interval);

                // If pair of v is not considered so far
                // (v, pairV[V]) is not yet explored edge.
                if(dist[pairV[nv]] == INF) {
                    // Consider the pair and add it to queue
                    dist[pairV[nv]] = dist[idx] + 1;
                    Q.push(pairV[nv]);
                }
            }
        }
    }
    // If we could come back to NIL using alternating path of distinct
    // vertices then there is an augmenting path
    return (dist[NIL] != INF);
}

bool Matcher::findMaximumMatching() {
    for(int idx = 0; idx < arity; idx++) {
        if(pairU[idx] != NIL && scope[idx]->containsValue(domainValue(pairU[idx])))
            continue;
        if(pairU[idx] != NIL)
            pairV[pairU[idx]] = NIL;
        pairU[idx] = NIL;
    }
    for(int u = 0; u < arity; u++) pairU[u] = NIL;
    for(int v = 0; v < interval; v++) pairV[v] = NIL;

    for(int idx = 0; idx < arity; idx++) {
        if(scope[idx]->size() == 1) {
            int nv     = normalizedValue(scope[idx]->value());
            pairV[nv]  = idx;
            pairU[idx] = nv;
        }
    }


    // Initialize result
    int result = 0;

    while(bfs()) {
        // Keep updating the result while there is an augmenting path.
        // Find a free vertex
        for(int idx = 0; idx < arity; idx++) {
            // If current vertex is free and there is
            // an augmenting path from current vertex
            if(pairU[idx] == NIL && dfs(idx))
                result++;
        }
    }
    return result == arity;
}