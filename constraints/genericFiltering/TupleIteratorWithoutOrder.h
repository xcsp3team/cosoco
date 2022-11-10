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
    explicit TupleIteratorWithoutOrder(vec<Variable *> &sc) : TupleIterator(sc) { positions.growTo(arity); }

    void setFirstTuple(vec<int> &first) override;

    void setFirstTuple(vec<int> &tuple, int posx, int idv) override;

    void setMinimalTuple() override;

    void setMinimalTuple(int posx, int idv) override;

    bool computeNextTuple() override;
};
}   // namespace Cosoco

#endif   // COSOCO_TUPLEMANAGERWITHOUTORDER_H
