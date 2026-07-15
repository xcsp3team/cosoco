#ifndef DOMAIN_H
#define DOMAIN_H


#include "mtl/LinkedSet.h"
#include "mtl/LinkedSetIterator.h"
#include "mtl/Vec.h"
#include "utils/Verbose.h"

namespace Cosoco {

class AbstractDomain {
    friend class Variable;

    friend class Solver;


   public:
    virtual ~AbstractDomain() = default;
    vec<int> nAssignments;

    // Constructors and initialisation
    explicit AbstractDomain(int sz) { }


    void delayedConstruction(int nbVars) { }


    // Virtual Method conversion id to value
    virtual int toIdv(int v) = 0;

    virtual int toVal(int idv) = 0;

    virtual void delIdv(int idv, int level) = 0;

    virtual void delVal(int v, int level) = 0;


    virtual void reduceTo(int idv, int level) = 0;

    virtual bool isIdentical(AbstractDomain &d) = 0;

    bool isConnex() { return maximum() - minimum() + 1 == size(); }

    virtual void reinitialize() = 0;


    virtual int valueAtPosition(int pos) = 0;


    virtual bool isBoolean() = 0;
    // inline const int getPosition(int idx) { return idvs.getPosition(idx); }

    virtual inline bool isIndexesAreValues() { return false; }


    // Method related to presence of value
    virtual bool containsValue(int v) = 0;


    virtual bool containsIdv(int idv) = 0;


    inline bool isUniqueValue(int v) { return size() == 1 && containsValue(v); }


    virtual int minimum() = 0;

    virtual int maximum() = 0;


    virtual int firstId() = 0;

    virtual int operator[](const int i) = 0;


    virtual int lastId() = 0;

    virtual int lastRemoved() = 0;

    virtual int prevRemoved(int id) = 0;


    virtual int nextIdv(int currentIdv) = 0;

    virtual int prevIdv(int currentIdv) = 0;


    // Methods related to size
    virtual bool isEmpty() = 0;

    virtual int size() = 0;

    virtual int maxSize() = 0;


    // Access method


    virtual void restoreLimit(int level) = 0;

    virtual int  lastRemovedLevel()                = 0;
    virtual bool isLimitRecordedAtLevel(int level) = 0;
    virtual void recordLimit(int level)            = 0;

    // Display
    virtual void display() = 0;

    virtual size_t hash() = 0;

    virtual bool equals(AbstractDomain *d) = 0;
};


class Domain : public AbstractDomain {
   protected:
    LinkedSet idvs;   // the set of elements

   public:
    // using const_iterator = LinkedSetIterator;
    using iterator = LinkedSetIterator;
    // using const_reverse_iterator = LinkedSetIterator;
    using reverse_iterator = LinkedSetIterator;

    explicit Domain(int sz) : AbstractDomain(sz), idvs(sz, true) { }
    // Methods related to deletion of values
    void delIdv(int idv, int level) override { idvs.del(idv, level); }

    void delVal(int v, int level) override { idvs.del(toIdv(v), level); }   // TODO Check range !!!!!!

    void reduceTo(int idv, int level) override { idvs.reduceTo(idv, level); }

    void reinitialize() override { idvs.fill(); }

    int valueAtPosition(int pos) override { return toVal(idvs[pos]); }

    bool isBoolean() override { return maxSize() == 2 && minimum() == 0 && maximum() == 1; }

    bool isIdentical(AbstractDomain &d) override {
        if(maxSize() != d.maxSize())
            return false;
        for(int i = 0; i < maxSize(); i++)
            if(d.containsValue(toVal(idvs[i])) == false)
                return false;
        return true;
    }

    bool containsValue(int v) override {
        int idv = toIdv(v);
        return idv == -1 ? false : idvs.contains(idv);
    }

    bool containsIdv(int idv) override { return idvs.contains(idv); }

    int minimum() override {
        assert(size() > 0);
        return toVal(idvs.first());
    }


    int maximum() override {
        assert(size() > 0);
        return toVal(idvs.last());
    }

    int firstId() override { return idvs.first(); }


    int lastId() override { return idvs.last(); }

    int lastRemoved() override { return idvs.lastRemoved(); }

    int prevRemoved(int id) override { return idvs.prevRemoved(id); }


    int nextIdv(int currentIdv) override { return idvs.next(currentIdv); }

    int prevIdv(int currentIdv) override { return idvs.prev(currentIdv); }


    bool isEmpty() override { return idvs.isEmpty(); }


    int size() override { return idvs.size(); }


    int maxSize() override { return idvs.maxSize(); }


    int operator[](const int i) { return idvs[i]; }

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

    void restoreLimit(int level) override { idvs.restoreLimit(level); }

    int lastRemovedLevel() override { return idvs.lastRemovedLevel(); }

    bool isLimitRecordedAtLevel(int level) override { return idvs.isLimitRecordedAtLevel(level); }

    void recordLimit(int level) override { idvs.recordLimit(level); }


    void display() override {
        printf("{");
        for(int idv = idvs.first(); idv != -1; idv = idvs.next(idv)) printf("%d ", toVal(idv));
    }
};
}   // namespace Cosoco

#endif
