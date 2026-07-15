
// Created by audemard on 13/07/2026.
//

#ifndef COSOCO_DOMAINBINARY_H
#define COSOCO_DOMAINBINARY_H
#include <cstddef>
#include <iterator>

#include "Domain.h"
namespace Cosoco {
class DomainBinary : public AbstractDomain {
   protected:
    int removed;
    int level1;
    int value0, value1;
    int sz;


   public:
    template <bool IsConst>
    class Iterator;
    using iterator       = Iterator<false>;
    using const_iterator = Iterator<true>;

    iterator       begin();
    iterator       end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

    DomainBinary(int v1, int v2) : AbstractDomain(2) {
        value0  = v1;
        value1  = v2;
        removed = -1;
        level1  = -1;
        sz      = 2;
    }
    int toIdv(int v) { return v == value0 ? 0 : 1; }

    int toVal(int idv) { return idv == 0 ? value0 : value1; }

    void delIdv(int idv, int l) {
        sz--;
        if(sz == 0)
            return;
        removed = idv;
        level1  = l;
    }

    void delVal(int v, int l) { delIdv(toIdv(v), l); }

    void reduceTo(int idv, int l) { delIdv(toIdv(idv), l); }

    bool isIdentical(AbstractDomain& d) {
        DomainBinary* db = dynamic_cast<DomainBinary*>(&d);
        if(db == nullptr)
            return false;
        return db->value0 == value0 && db->value1 == value1;
    }

    void reinitialize() {
        level1  = -1;
        sz      = 2;
        removed = -1;
    }
    int valueAtPosition(int pos) { return pos == 0 ? value0 : value1; }

    bool isBoolean() { return value0 == 0 && value1 == 1; }

    bool containsValue(int v) { return containsIdv(toIdv(v)); }

    bool containsIdv(int idv) { return sz == 2 || (removed != idv && sz == 1); }

    int minimum() { return sz == 2 || removed == 1 ? toVal(0) : toVal(1); }
    int maximum() { return sz == 2 || removed == 0 ? toVal(1) : toVal(0); }

    int firstId() {
        assert(sz > 0);
        return toIdv(minimum());
    }
    int lastId() {
        assert(sz > 0);
        return toIdv(maximum());
    }
    int lastRemoved() { }

    int prevRemoved(int id) { }

    int nextIdv(int currentIdv) {
        if(currentIdv == 0 && sz == 2)
            return 1;
        return -1;
    }

    int prevIdv(int currentIdv) {
        if(currentIdv == 1 && sz == 2)
            return 0;
        return -1;
    }

    bool isEmpty() { return sz == 0; }

    int size() { return sz; }

    int maxSize() { return 2; }

    void restoreLimit(int level) {
        if(level1 == level) {
            level1  = -1;
            sz      = 2;
            removed = -1;
        }
    }

    int lastRemovedLevel() { return level1; }

    bool isLimitRecordedAtLevel(int level) { return level1 == level; }

    void recordLimit(int level) { level1 = level; }

    void display() { }

    size_t hash() {
        std::hash<std::string> h;
        return h(std::to_string(value0) + " " + std::to_string(value1));
    }

    bool equals(AbstractDomain* d) {
        auto* dr = dynamic_cast<DomainBinary*>(d);
        if(dr == nullptr)
            return false;
        return value0 == dr->value0 && value1 == dr->value1;
    }

    int operator[](const int i) {
        if(sz == 2)
            return i;
        return 1 - removed;
    }
};

template <bool IsConst>
class DomainBinary::Iterator {
   public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = int;
    using difference_type   = std::ptrdiff_t;
    using DomPtr            = std::conditional_t<IsConst, const DomainBinary*, DomainBinary*>;
    using reference         = std::conditional_t<IsConst, const int&, int&>;
    using pointer           = std::conditional_t<IsConst, const int*, int*>;

    Iterator() = default;
    Iterator(DomPtr dom, int pos) : domain(dom), pos_(pos) { }

    // Déréférencement : traduit la position logique (0..sz-1) en value0/value1
    reference operator*() const { return deref(pos_); }
    pointer   operator->() const { return &deref(pos_); }
    reference operator[](difference_type n) const { return deref(pos_ + static_cast<int>(n)); }

    // Incrémentation / décrémentation
    Iterator& operator++() {
        ++pos_;
        return *this;
    }
    Iterator operator++(int) {
        Iterator tmp(*this);
        ++pos_;
        return tmp;
    }
    Iterator& operator--() {
        --pos_;
        return *this;
    }
    Iterator operator--(int) {
        Iterator tmp(*this);
        --pos_;
        return tmp;
    }

    // Arithmétique
    Iterator& operator+=(difference_type n) {
        pos_ += static_cast<int>(n);
        return *this;
    }
    Iterator& operator-=(difference_type n) {
        pos_ -= static_cast<int>(n);
        return *this;
    }
    friend Iterator operator+(Iterator it, difference_type n) {
        it += n;
        return it;
    }
    friend Iterator operator+(difference_type n, Iterator it) {
        it += n;
        return it;
    }
    friend Iterator operator-(Iterator it, difference_type n) {
        it -= n;
        return it;
    }
    friend difference_type operator-(const Iterator& a, const Iterator& b) {
        return static_cast<difference_type>(a.pos_) - static_cast<difference_type>(b.pos_);
    }

    // Comparaisons
    friend bool operator==(const Iterator& a, const Iterator& b) { return a.pos_ == b.pos_; }
    friend bool operator!=(const Iterator& a, const Iterator& b) { return a.pos_ != b.pos_; }
    friend bool operator<(const Iterator& a, const Iterator& b) { return a.pos_ < b.pos_; }
    friend bool operator>(const Iterator& a, const Iterator& b) { return a.pos_ > b.pos_; }
    friend bool operator<=(const Iterator& a, const Iterator& b) { return a.pos_ <= b.pos_; }
    friend bool operator>=(const Iterator& a, const Iterator& b) { return a.pos_ >= b.pos_; }

   private:
    // Logical member
    reference deref(int pos) const {
        if(domain->sz == -1)
            return pos;

        return (domain->removed == domain->value0) ? 1 : 0;
    }

    DomPtr domain = nullptr;
    int    pos_   = 0;
};

inline DomainBinary::iterator       DomainBinary::begin() { return iterator(this, 0); }
inline DomainBinary::iterator       DomainBinary::end() { return iterator(this, sz); }
inline DomainBinary::const_iterator DomainBinary::begin() const { return const_iterator(this, 0); }
inline DomainBinary::const_iterator DomainBinary::end() const { return const_iterator(this, sz); }
inline DomainBinary::const_iterator DomainBinary::cbegin() const { return const_iterator(this, 0); }
inline DomainBinary::const_iterator DomainBinary::cend() const { return const_iterator(this, sz); }

}   // namespace Cosoco
// namespace Cosoco
#endif   // COSOCO_DOMAINBINARY_H
