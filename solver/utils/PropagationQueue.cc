//
// Created by audemard on 18/03/25.
//

#include "PropagationQueue.h"
using namespace Cosoco;

PropagationQueue::PropagationQueue(int sz) : variablesInQueue(sz) {
    buckets.growTo(NB_BUCKETS + 1);
    variablesInformation.growTo(sz, Data());
}

static inline double drand(double& seed) {
    seed *= 1389796;
    int q = (int)(seed / 2147483647);
    seed -= (double)q * 2147483647;
    return seed / 2147483647;
}

// Returns a random integer 0 <= x < size. Seed must never be 0.
static inline int irand(double& seed, int size) { return (int)(drand(seed) * size); }


Variable* PropagationQueue::pickInQueue() {
    for(auto& b : buckets) {
        if(b.size() > 0) {
            int       p                           = irand(seed, b.size());
            Variable* tmp                         = b[p];
            Variable* x                           = b.last();
            b[p]                                  = x;
            variablesInformation[x->idx].position = p;
            b.pop_();
            variablesInQueue.del(tmp->idx);
            return tmp;
        }
    }
    return nullptr;
}

int PropagationQueue::findBucket(int sz) {
    if(sz <= 10)
        return sz;
    if(sz <= 20)
        return 11;
    if(sz <= 50)
        return 12;
    return 13;
}

void PropagationQueue::fill(vec<Variable*>& variables) {
    for(Variable* x : variables) add(x);
}


void PropagationQueue::add(Variable* var) {
    if(variablesInQueue.contains(var->idx)) {
        int p        = variablesInformation[var->idx].position;
        int n_bucket = variablesInformation[var->idx].bucket;
        assert(buckets[n_bucket][p] == var);
        Variable* x                           = buckets[n_bucket].last();
        buckets[n_bucket][p]                  = x;
        variablesInformation[x->idx].position = p;
        buckets[n_bucket].pop_();
    }
    variablesInQueue.add(var->idx);
    int n_bucket = findBucket(var->size());
    buckets[n_bucket].push(var);
    variablesInformation[var->idx].position = buckets[n_bucket].size() - 1;
    variablesInformation[var->idx].bucket   = n_bucket;
}


void PropagationQueue::clear() {
    variablesInQueue.clear();
    for(auto& b : buckets) b.clear();
}

int PropagationQueue::size() { return variablesInQueue.size(); }