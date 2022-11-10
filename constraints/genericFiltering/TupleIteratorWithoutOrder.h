#ifndef COSOCO_TUPLEMANAGERWITHOUTORDER_H
#define COSOCO_TUPLEMANAGERWITHOUTORDER_H

/**
 * this version allows to iterate all valid tuples wrt current interpretation
 * tuples are not iterated wrt to lexicographic order
 * This is due to domain encodings using sparseset
 *
 */

#include "TupleIterator.h"
#include "core/Variable.h"
#include "mtl/Vec.h"

namespace Cosoco {
class TupleIteratorWithoutOrder : public TupleIterator {
   protected:
    vec<int> positions;

   public:
    TupleIteratorWithoutOrder(vec<Variable *> &sc) : TupleIterator(sc) { positions.growTo(arity); }

    virtual void setFirstTuple(vec<int> &first) override;

    virtual void setFirstTuple(vec<int> &tuple, int posx, int idv) override;

    virtual void setMinimalTuple() override;

    virtual void setMinimalTuple(int posx, int idv) override;

    virtual bool computeNextTuple() override;
};
}   // namespace Cosoco

#endif   // COSOCO_TUPLEMANAGERWITHOUTORDER_H
