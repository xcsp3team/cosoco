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
    bool      eq;

    Tuple(Variable *_x, int _idv, bool e = false) : x(_x), idv(_idv), eq(e) { }
    Tuple(const Tuple &t) : x(t.x), idv(t.idv), eq(t.eq) { }
    Tuple &operator=(const Tuple &other) {
        // Guard self assignment
        if(this == &other)
            return *this;
        x   = other.x;
        idv = other.idv;
        eq  = other.eq;
        return *this;
    }
    // friend std::ostream &operator<<(std::ostream &stream, Tuple const &tuple);
};


inline bool operator==(const Tuple &a, const Tuple &b) { return a.x == b.x && a.idv == b.idv && a.eq == b.eq; }
inline bool operator!=(const Tuple &a, const Tuple &b) { return a.x != b.x || a.idv != b.idv || a.eq != b.eq; }

struct cmpTuple {
    bool operator()(const Tuple &a, const Tuple &b) const {
        if(a.x->idx < b.x->idx)
            return true;
        if(a.x->idx > b.x->idx)
            return false;
        if(a.eq == false && b.eq)
            return true;
        if(a.eq && b.eq == false)
            return true;
        return a.idv < b.idv;
    }
};
}   // namespace Cosoco
#endif   // COSOCO_TUPLE_H
