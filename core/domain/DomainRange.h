#ifndef DOMAINRANGE_H
#define DOMAINRANGE_H

#include "Domain.h"

namespace Cosoco {


class Range {
   public :
    int min, max;
    Range(int mn, int mx) {
        if(mn < mx) {
            min = mn;max = mx;
        } else {
            min = mx;
            max = mn;
        }
    }
    Range negRange() const {
        return {-max, -min};
    }

    int contains(int v) const {
        return min <= v and v <= max;
    }

    Range absRange() const {
        return {contains(0) ? 0 : std::min(abs(min), abs(max)), std::max(abs(min), abs(max))};
    }

   Range addRange(Range r2) const {
        return {min + r2.min, max + r2.max};
    }
};

class DomainRange : public Domain {
   protected:
    int min, max;

   public:
    // Constructors
    DomainRange(int mn, int mx) : Domain(mx - mn + 1), min(mn), max(mx) { }

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
};
}   // namespace Cosoco
#endif
