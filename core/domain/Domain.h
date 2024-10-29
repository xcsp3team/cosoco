#ifndef DOMAIN_H
#define DOMAIN_H


#include "mtl/LinkedSet.h"
#include "mtl/LinkedSetIterator.h"
#include "mtl/Vec.h"
#include "utils/Verbose.h"

namespace Cosoco {

class Domain {
    friend class Variable;

    friend class Solver;

   protected:
    LinkedSet idvs;   // the set of elements

   public:
    vec<int> nAssignments;
    // using const_iterator = LinkedSetIterator;
    using iterator = LinkedSetIterator;
    // using const_reverse_iterator = LinkedSetIterator;
    using reverse_iterator = LinkedSetIterator;

    // Constructors and initialisation
    Domain(int sz) : idvs(sz, true) { }
    virtual ~Domain() = default;


    void delayedConstruction(int nbVars) { }


    // Virtual Method conversion id to value
    virtual int toIdv(int v) = 0;

    virtual int toVal(int idv) = 0;

    // Methods related to deletion of values
    inline void delIdv(int idv, int level) { idvs.del(idv, level); }


    inline void delVal(int v, int level) { idvs.del(toIdv(v), level); }   // TODO Check range !!!!!!


    inline void reduceTo(int idv, int level) { idvs.reduceTo(idv, level); }

    bool isIdentical(Domain &d) {
        if(maxSize() != d.maxSize())
            return false;
        for(int i = 0; i < maxSize(); i++)
            if(d.containsValue(toVal(idvs[i])) == false)
                return false;
        return true;
    }

    void reinitialize() { idvs.fill(); }


    inline int valueAtPosition(int pos) { return toVal(idvs[pos]); }


    inline bool isBoolean() { return maxSize() == 2 && minimum() == 0 && maximum() == 1; }
    // inline const int getPosition(int idx) { return idvs.getPosition(idx); }

    virtual inline bool isIndexesAreValues() { return false; }


    // Method related to presence of value
    inline bool containsValue(int v) {
        int idv = toIdv(v);
        return idv == -1 ? false : idvs.contains(idv);
    }


    inline bool containsIdv(int idv) { return idvs.contains(idv); }


    inline bool isUniqueValue(int v) { return size() == 1 && containsValue(v); }


    inline int minimum() {
        assert(size() > 0);
        return toVal(idvs.first());
    }


    inline int maximum() {
        assert(size() > 0);
        return toVal(idvs.last());
    }


    inline int firstId() { return idvs.first(); }


    inline int lastId() { return idvs.last(); }

    inline int lastRemoved() { return idvs.lastRemoved(); }

    inline int prevRemoved(int id) { return idvs.prevRemoved(id); }


    // return the next valid index in the domain, -1 if none exists (in such a case, idx is equal to the max
    /*inline int nextValidIndex(int currentIdv) {
        assert(size() > 0);
        int nSlots = maximum();
        if(toIdv(nSlots) == currentIdv) return -1;
        for(int idv = currentIdv + 1; idv < maxSize(); idv++) {
            if(containsIdv(idv))
                return idv;
        }
        return -1;
    }*/

    int nextIdv(int currentIdv) { return idvs.next(currentIdv); }


    // Methods related to size
    inline bool isEmpty() { return idvs.isEmpty(); }


    inline int size() { return idvs.size(); }


    inline int maxSize() { return idvs.maxSize(); }


    // Access method
    inline int operator[](const int i) { return idvs[i]; }


    // begin/end functions to use for each aka c++
    iterator begin() { return idvs.begin(); }

    // const_iterator cbegin() const { return idvs.cbegin(); }
    iterator end() { return idvs.end(); }
    // const_iterator cend() const { return idvs.cend(); }

    // rbegin/rend functions to reverse traversal
    reverse_iterator rbegin() { return idvs.rbegin(); }

    // const_reverse_iterator rbegin() const { return idvs.crbegin(); }
    reverse_iterator rend() { return idvs.rend(); }
    // const_reverse_iterator rend() const { return idvs.crend(); }


    inline void restoreLimit(int level) { idvs.restoreLimit(level); }

    inline int lastRemovedLevel() { return idvs.lastRemovedLevel(); }

    // Display
    virtual void display() {
        printf("{");
        for(int idv = idvs.first(); idv != -1; idv = idvs.next(idv)) printf("%d ", toVal(idv));
    }

    virtual size_t hash() = 0;

    virtual bool equals(Domain *d) = 0;
};

}   // namespace Cosoco

#endif
