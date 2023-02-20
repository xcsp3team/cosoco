//
// Created by audemard on 20/02/23.
//

#ifndef COSOCO_TUPLE_H
#define COSOCO_TUPLE_H
#include "solver/Solver.h"
namespace Cosoco {
class Tuple {
   public:
    Variable *x;
    int       idv;

    Tuple(Variable *_x, int _idv) : x(_x), idv(_idv) { }
    Tuple(const Tuple &t) : x(t.x), idv(t.idv) { }
    Tuple &operator=(const Tuple &other) {
        // Guard self assignment
        if(this == &other)
            return *this;
        x   = other.x;
        idv = other.idv;
        return *this;
    }
};

inline bool operator==(const Tuple &a, const Tuple &b) { return a.x == b.x && a.idv == b.idv; }
inline bool operator!=(const Tuple &a, const Tuple &b) { return a.x != b.x or a.idv != b.idv; }

struct cmpTuple {
    bool operator()(const Tuple &a, const Tuple &b) const {
        if(a.x->idx < b.x->idx)
            return true;
        if(a.x->idx > b.x->idx)
            return false;
        return a.idv < b.idv;
    }
};
}   // namespace Cosoco
#endif   // COSOCO_TUPLE_H
