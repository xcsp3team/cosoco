#ifndef DOMAIN_H
#define DOMAIN_H


#include "mtl/LinkedSet.h"
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
    virtual int toIdv(int v) const = 0;

    virtual int toVal(int idv) const = 0;

    virtual void delIdv(int idv, int level) = 0;

    virtual void delVal(int v, int level) = 0;


    virtual void reduceTo(int idv, int level) = 0;

    virtual bool isIdentical(AbstractDomain& d) = 0;

    bool isConnex() { return maximum() - minimum() + 1 == size(); }

    virtual void reinitialize() = 0;


    virtual int valueAtPosition(int pos) const = 0;
    virtual int indexAtPosition(int pos) const = 0;

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

    int operator[](const int i) { return indexAtPosition(i); }


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

    virtual bool equals(AbstractDomain* d) = 0;

    class iterator {
       public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = int;
        using difference_type   = std::ptrdiff_t;
        using pointer           = void;
        using reference         = int;   // par valeur : le type réel de stockage diffère selon la fille

        iterator() = default;
        iterator(AbstractDomain* dom, int pos) : domain(dom), pos_(pos) { }

        int operator*() { return domain->indexAtPosition(pos_); }
        int operator[](difference_type n) { return domain->indexAtPosition(pos_ + static_cast<int>(n)); }

        iterator& operator++() {
            ++pos_;
            return *this;
        }
        iterator operator++(int) {
            iterator tmp(*this);
            ++pos_;
            return tmp;
        }
        iterator& operator--() {
            --pos_;
            return *this;
        }
        iterator operator--(int) {
            iterator tmp(*this);
            --pos_;
            return tmp;
        }

        iterator& operator+=(difference_type n) {
            pos_ += static_cast<int>(n);
            return *this;
        }
        iterator& operator-=(difference_type n) {
            pos_ -= static_cast<int>(n);
            return *this;
        }
        friend iterator operator+(iterator it, difference_type n) {
            it += n;
            return it;
        }
        friend iterator operator+(difference_type n, iterator it) {
            it += n;
            return it;
        }
        friend iterator operator-(iterator it, difference_type n) {
            it -= n;
            return it;
        }
        friend difference_type operator-(const iterator& a, const iterator& b) {
            return static_cast<difference_type>(a.pos_) - static_cast<difference_type>(b.pos_);
        }

        friend bool operator==(const iterator& a, const iterator& b) { return a.pos_ == b.pos_; }
        friend bool operator!=(const iterator& a, const iterator& b) { return a.pos_ != b.pos_; }
        friend bool operator<(const iterator& a, const iterator& b) { return a.pos_ < b.pos_; }
        friend bool operator>(const iterator& a, const iterator& b) { return a.pos_ > b.pos_; }
        friend bool operator<=(const iterator& a, const iterator& b) { return a.pos_ <= b.pos_; }
        friend bool operator>=(const iterator& a, const iterator& b) { return a.pos_ >= b.pos_; }

       private:
        AbstractDomain* domain = nullptr;
        int             pos_   = 0;
    };

    iterator begin() const { return iterator(this, 0); }
    iterator end() { return iterator(this, size()); }

    iterator rbegin() { return iterator(this, size()); }
    iterator rend() const { return iterator(this, 0); }
};


class Domain : public AbstractDomain {
   protected:
    LinkedSet idvs;   // the set of elements

   public:
    explicit Domain(int sz) : AbstractDomain(sz), idvs(sz, true) { }
    // Methods related to deletion of values
    void delIdv(int idv, int level) override { idvs.del(idv, level); }

    void delVal(int v, int level) override { idvs.del(toIdv(v), level); }   // TODO Check range !!!!!!

    void reduceTo(int idv, int level) override { idvs.reduceTo(idv, level); }

    void reinitialize() override { idvs.fill(); }

    int valueAtPosition(int pos) override { return toVal(idvs[pos]); }

    int indexAtPosition(int pos) override { return idvs[pos]; }


    bool isBoolean() override { return maxSize() == 2 && minimum() == 0 && maximum() == 1; }

    bool isIdentical(AbstractDomain& d) override {
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
