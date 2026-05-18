#ifndef DOMAINSMALLVALUE_H
#define DOMAINSMALLVALUE_H

#include <XCSP3Variable.h>

#include <map>

#include "Domain.h"
#include "Utils.h"
#include "mtl/SparseSetMultiLevel.h"
#include "mtl/Vec.h"

namespace Cosoco {
// TODO limit for toIdvs and to Val
class DomainSmallValue : public Domain {
   protected:
    vec<int> values;
    vec<int> toIndex;
    int      offset;

   public:
    // Constructors
    explicit DomainSmallValue(const vec<int> &vals) : Domain(vals.size()) {
        offset  = vals[0];
        int max = vals.last();
        toIndex.growTo(max - offset + 1, -1);

        for(int i = 0; i < vals.size(); i++) {
            values.push(vals[i]);
            toIndex[vals[i] - offset] = i;
        }
        nAssignments.growTo(vals.size(), 0);
    }


    // Virtual Method conversion id to value
    int toIdv(int v) override {
        if(v - offset < 0 || v - offset >= toIndex.size())
            return -1;
        return toIndex[v - offset];
    }


    int toVal(int idv) override { return values[idv]; }

    bool isIndexesAreValues() override { return minimum() == 0 && maximum() == maxSize(); }

    size_t hash() override {
        std::string            s;
        std::hash<std::string> h;
        for(int v : values) s += std::to_string(v) + " ";
        return h(s);
    }

    bool equals(Domain *d) override {
        auto *dv = dynamic_cast<DomainSmallValue *>(d);
        if(dv == nullptr || values.size() != dv->values.size())
            return false;
        for(int i = 0; i < values.size(); i++)
            if(values[i] != dv->values[i])
                return false;
        return true;
    }
};
}   // namespace Cosoco
#endif