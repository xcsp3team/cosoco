#include "TupleIteratorWithoutOrder.h"

using namespace Cosoco;


bool TupleIteratorWithoutOrder::computeNextTuple() {
    for(int i = arity - 1; i >= 0; i--) {
        if(fixedPositions[i] || scope[i]->size() == 1)
            continue;

        int pos = positions[i];
        if(pos == scope[i]->size() - 1) {   // The last one
            current[i]   = scope[i]->domain[0];
            positions[i] = 0;
        } else {
            current[i] = scope[i]->domain.nextIdv(current[i]);
            positions[i]++;
            return true;
        }
    }
    return false;
}


void TupleIteratorWithoutOrder::setFirstTuple(vec<int> &first, int posx, int idv) {
    throw std::runtime_error("No ordered iteration when using sparse set domains");
}


void TupleIteratorWithoutOrder::setFirstTuple(vec<int> &first) {
    throw std::runtime_error("No ordered iteration when using sparse set domains");
}


void TupleIteratorWithoutOrder::setMinimalTuple() {
    hasNext = true;
    for(int i = 0; i < arity; i++) {
        current[i]        = scope[i]->domain[0];
        fixedPositions[i] = false;
        positions[i]      = 0;
    }
}


void TupleIteratorWithoutOrder::setMinimalTuple(int posx, int idv) {
    assert(scope[posx]->domain.containsIdv(idv));
    setMinimalTuple();
    fixId(posx, idv);
}
