#ifndef TUPLEMANAGER_H
#define TUPLEMANAGER_H

#include "core/Variable.h"
#include "mtl/Vec.h"


/**
 * this version allows to iterate all valid tuples wrt current interpretation
 * tuples are not iterated wrt to lexicographic order
 * This is due to domain encodings using sparseset
 *
 */

namespace Cosoco {

class TupleIterator {
   protected:
    vec<bool>        fixedPositions;
    int              arity;   // useless but good
    vec<int>         current;
    vec<int>         toReturn;
    vec<Variable *> &scope;
    bool             hasNext;

   public:
    TupleIterator(vec<Variable *> &sc) : scope(sc) {
        arity = scope.size();
        current.growTo(arity);
        toReturn.growTo(arity);
        fixedPositions.growTo(arity);
    }
    virtual ~TupleIterator() = default;

    virtual void setFirstTuple(vec<int> &first) = 0;

    virtual void setFirstTuple(vec<int> &tuple, int posx, int idv) = 0;

    virtual void setMinimalTuple() = 0;

    virtual void setMinimalTuple(int posx, int idv) = 0;

    virtual bool computeNextTuple() = 0;

    bool hasNextTuple() const { return hasNext; }

    vec<int> *nextTuple() {
        current.copyTo(toReturn);
        hasNext = computeNextTuple();
        return &toReturn;
    }

    void fixId(int posx, int idv) {
        current[posx]        = idv;
        fixedPositions[posx] = true;
    }
};
}   // namespace Cosoco

#endif /* TUPLEMANAGER_H */
