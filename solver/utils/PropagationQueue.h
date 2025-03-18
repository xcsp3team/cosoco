//
// Created by audemard on 18/03/25.
//

#ifndef PROPAGATIONQUEUE_H
#define PROPAGATIONQUEUE_H
#include <SparseSet.h>
#include <Variable.h>

namespace Cosoco {

class Data {
   public:
    int bucket;
    int position;
    Data(int bucket, int position, int idx) {
        this->bucket   = bucket;
        this->position = position;
    }

    Data() { }
};


#define NB_BUCKETS 13

// int buckets_size[NB_BUCKETS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 50};   // 0 is useless

class PropagationQueue {
   protected:
    SparseSet            variablesInQueue;
    vec<Data>            variablesInformation;
    vec<vec<Variable*> > buckets;
    double               seed = 123456;

   public:
    explicit PropagationQueue(int sz);
    Variable*  pickInQueue();
    void       add(Variable* var);
    void       clear();
    int        size();
    void       fill(vec<Variable*>& vars);
    static int findBucket(int sz);
};
}   // namespace Cosoco


#endif   // PROPAGATIONQUEUE_H
