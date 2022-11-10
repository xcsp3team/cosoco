#ifndef SPARSESETOFVARIABLES_H
#define SPARSESETOFVARIABLES_H


#include <assert.h>

#include <iostream>

#include "core/Constants.h"
#include "core/Variable.h"
#include "mtl/Vec.h"

namespace Cosoco {

class Variable;

/**
 * SparseSet
 * Store variables to avoid pointer dereference and array accesses
 *      * can manage insertion, deletion... in O(1)
 * See Preston Briggs and Linda Torczon.
 *     An Efficient Representation for Sparse Sets”, 1993.
 *
 */
class SparseSetOfVariables {
   protected:
    vec<int>        positions;   // Correspond to sparse
    vec<Variable *> elements;    // Correspond to dense
    int             limit;
    int             maxsz;

   public:
    // Constructor
    SparseSetOfVariables(int size, vec<Variable *> &vars, bool full = false) : maxsz(size) {
        positions.growTo(size);
        elements.growTo(size);
        assert(vars.size() == size);
        for(int i = 0; i < size; i++) {
            assert(vars[i]->idx == i);   // Be careful
            positions[i] = i;
            elements[i]  = vars[i];
        }
        limit = full ? size : 0;
    }


    // Fill and clear methods
    inline void clear() { limit = 0; }
    inline void fill() { limit = maxsz; }


    // Questions about size methods
    inline bool isEmpty() const { return (limit == 0); }
    inline int  size() const { return limit; }
    inline int  maxSize() const { return maxsz; }


    // Insert and delete methods
    inline void add(Variable *x) {
        int k = x->idx;
        assert(k < maxSize());   // verify that we are bound safe
        if(contains(x))          // already there
            return;
        Variable *tmp = elements[limit];
        int       pos = positions[k];

        positions[tmp->idx] = pos;
        elements[pos]       = tmp;

        elements[limit] = x;
        positions[k]    = limit;
        limit++;
    }

    inline void del(Variable *x) {
        int k = x->idx;
        assert(k < maxSize());
        if(!contains(x))
            return;
        limit--;
        Variable *tmp       = elements[limit];
        int       pos       = positions[k];
        positions[tmp->idx] = pos;
        elements[pos]       = tmp;
        elements[limit]     = x;
        positions[k]        = limit;
    }

    // Access and contains method
    inline bool contains(Variable *x) const {
        assert(x->idx <= maxSize());
        return positions[x->idx] < limit;
    }
    inline Variable *operator[](const int i) { return elements[i]; }

    // Display method
    void display() {
        printf("[");
        for(int i = 0; i < size(); i++) printf(" %s  ", elements[i]->name());
        printf("]");

        printf(" // [");
        for(int i = size(); i < maxSize(); i++) {
            printf("%s %s %s ", KRED, elements[i]->name(), KNRM);
        }
        printf("]");
    }
};
};   // namespace Cosoco


#endif /* SPARSESETOFVARIABLES_H */
