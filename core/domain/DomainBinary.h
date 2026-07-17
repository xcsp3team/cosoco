
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


    DomainBinary(int v1, int v2) : AbstractDomain(2) {
        value0  = v1;
        value1  = v2;
        removed = -1;
        level1  = -1;
        sz      = 2;
    }
    int toIdv(int v) override { return v == value0 ? 0 : 1; }

    int toVal(int idv) override { return idv == 0 ? value0 : value1; }

    void delIdv(int idv, int l) {
        sz--;
        assert(sz >= 0);
        if(sz == 0)
            return;
        removed = idv;
        level1  = l;
    }

    void delVal(int v, int l) override { delIdv(toIdv(v), l); }

    void reduceTo(int idv, int l) override { delIdv(toIdv(idv), l); }

    bool isIdentical(AbstractDomain& d) {
        DomainBinary* db = dynamic_cast<DomainBinary*>(&d);
        if(db == nullptr)
            return false;
        return db->value0 == value0 && db->value1 == value1;
    }

    void reinitialize() override {
        level1  = -1;
        sz      = 2;
        removed = -1;
    }
    int valueAtPosition(int pos) override { return sz == 2 || removed == 1 ? toVal(pos) : value1; }

    int indexAtPosition(int pos) override { return sz == 2 || removed == 1 ? pos : 1; }

    bool isBoolean() override { return value0 == 0 && value1 == 1; }

    bool containsValue(int v) override { return containsIdv(toIdv(v)); }

    bool containsIdv(int idv) override { return sz == 2 || (removed != idv && sz == 1); }

    int minimum() override { return sz == 2 || removed == 1 ? toVal(0) : toVal(1); }

    int maximum() override { return sz == 2 || removed == 0 ? toVal(1) : toVal(0); }

    int firstId() override {
        assert(sz > 0);
        return toIdv(minimum());
    }
    int lastId() override {
        assert(sz > 0);
        return toIdv(maximum());
    }
    int lastRemoved() override { assert(false); }

    int prevRemoved(int id) override { assert(false); }

    int nextIdv(int currentIdv) override {
        if(currentIdv == 0 && sz == 2)
            return 1;
        return -1;
    }

    int prevIdv(int currentIdv) override {
        if(currentIdv == 1 && sz == 2)
            return 0;
        return -1;
    }

    bool isEmpty() override { return sz == 0; }

    int size() override { return sz; }

    int maxSize() override { return 2; }

    void restoreLimit(int level) override {
        if(level1 == level) {
            level1  = -1;
            sz      = 2;
            removed = -1;
        }
    }

    int lastRemovedLevel() override { return level1; }

    bool isLimitRecordedAtLevel(int level) override { return level1 == level; }

    void recordLimit(int level) override { level1 = level; }

    void display() override { }

    size_t hash() override {
        std::hash<std::string> h;
        return h(std::to_string(value0) + " " + std::to_string(value1));
    }

    bool equals(AbstractDomain* d) override {
        auto* dr = dynamic_cast<DomainBinary*>(d);
        if(dr == nullptr)
            return false;
        return value0 == dr->value0 && value1 == dr->value1;
    }
};
}   // namespace Cosoco
// namespace Cosoco
#endif   // COSOCO_DOMAINBINARY_H
