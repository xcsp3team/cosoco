#ifndef DOMAIN_H
#define DOMAIN_H


#include "mtl/LinkedSet.h"
#include "mtl/LinkedSetIterator.h"
#include "mtl/LinkedSetOpt.h"
#include "mtl/Vec.h"
#include "utils/Verbose.h"

namespace Cosoco {

class Domain {
    friend class Variable;

    friend class Solver;

   protected:
    LinkedSet idvs;   // the set of elements

   public:
    virtual ~Domain() = default;
    vec<int> nAssignments;
    using iterator         = LinkedSetIterator;
    using reverse_iterator = LinkedSetIterator;
    bool isOptimized;
    // ---------- Constructors and initialisation
    Domain(int sz, bool isOpt = false) : idvs(sz, true, isOpt), isOptimized(isOpt) { }
    void reinitialize() { idvs.fill(); }
    void delayedConstruction(int nbVars) { }

    // ---------- Virtual Method conversion id to value
    virtual int toIdv(int v)   = 0;
    virtual int toVal(int idv) = 0;

    // ---------- Methods related to deletion of values
    void delIdv(int idv, int level) { idvs.del(idv, level); }
    void delVal(int v, int level) { idvs.del(toIdv(v), level); }
    void reduceTo(int idv, int level) { idvs.reduceTo(idv, level); }

    // ---------- Questions related to domain
    bool isIdentical(Domain &d) {
        if(maxSize() != d.maxSize())
            return false;
        for(int i = 0; i < maxSize(); i++)
            if(d.containsValue(toVal(idvs[i])) == false)
                return false;
        return true;
    }

    bool         isConnex() { return maximum() - minimum() + 1 == size(); }
    bool         isBoolean() { return maxSize() == 2 && minimum() == 0 && maximum() == 1; }
    virtual bool isIndexesAreValues() { return false; }


    // ---------- Method related to values
    int  valueAtPosition(int pos) { return toVal(idvs[pos]); }
    bool isUniqueValue(int v) { return size() == 1 && containsValue(v); }

    bool containsValue(int v) {
        int idv = toIdv(v);
        return idv == -1 ? false : idvs.contains(idv);
    }

    bool containsIdv(int idv) { return idvs.contains(idv); }

    int minimum() {
        assert(size() > 0);
        return toVal(idvs.first());
    }

    int maximum() {
        assert(size() > 0);
        return toVal(idvs.last());
    }

    int firstId() { return idvs.first(); }
    int lastId() { return idvs.last(); }

    int lastRemoved() { return idvs.lastRemoved(); }
    int prevRemoved(int id) { return idvs.prevRemoved(id); }


    int nextIdv(int currentIdv) { return idvs.next(currentIdv); }
    int prevIdv(int currentIdv) { return idvs.prev(currentIdv); }


    // ---------- Methods related to size
    bool isEmpty() { return idvs.isEmpty(); }
    int  size() { return idvs.size(); }
    int  maxSize() { return idvs.maxSize(); }


    // ---------- Access method
    int operator[](const int i) { return idvs[i]; }


    // begin/end functions to use for each aka c++
    iterator begin() { return idvs.begin(); }
    iterator end() { return idvs.end(); }

    reverse_iterator rbegin() { return idvs.rbegin(); }
    reverse_iterator rend() { return idvs.rend(); }


    void restore(int level) { idvs.restoreLimit(level); }
    int  lastRemovedLevel() { return idvs.lastRemovedLevel(); }

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
