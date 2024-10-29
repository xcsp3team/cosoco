#ifndef DOMAINRANGE_H
#define DOMAINRANGE_H

#include "core/domain/Domain.h"

namespace Cosoco {


class Range {
   public:
    int min, max;
    Range(int mn, int mx) {
        if(mn < mx) {
            min = mn;
            max = mx;
        } else {
            min = mx;
            max = mn;
        }
    }
    Range negRange() const { return {-max, -min}; }

    int contains(int v) const { return min <= v and v <= max; }

    Range absRange() const { return {contains(0) ? 0 : std::min(abs(min), abs(max)), std::max(abs(min), abs(max))}; }

    Range addRange(Range r2) const { return {min + r2.min, max + r2.max}; }
};

class DomainRange : public Domain {
   protected:
    int min, max;

   public:
    // Constructors
    DomainRange(int mn, int mx) : Domain(mx - mn + 1), min(mn), max(mx) { nAssignments.growTo(mx - mn + 1, 0); }

    // Virtual Method conversion id to value

    int toIdv(int v) override {
        if(v < min || v > max)
            return -1;
        return v - min;
    }

    int toVal(int idv) override {
        assert(idv >= 0 && idv < idvs.maxSize());
        return min + idv;
    }

    size_t hash() override {
        std::string            s = std::to_string(min) + " " + std::to_string(max) + " " + std::to_string(INT_MAX);
        std::hash<std::string> h;
        return h(s);
    }

    bool equals(Domain *d) override {
        auto *dr = dynamic_cast<DomainRange *>(d);
        if(dr == nullptr)
            return false;
        return min == dr->min && max == dr->max;
    }
};
}   // namespace Cosoco
#endif
