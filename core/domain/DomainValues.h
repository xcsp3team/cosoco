#ifndef DOMAINVALUE_H
#define DOMAINVALUE_H

#include "Domain.h"
#include "mtl/SparseSetMultiLevel.h"
#include "mtl/Vec.h"

namespace Cosoco {
// TODO limit for toIdvs and to Val
class DomainValue : public Domain {
   protected:
    vec<int>           values;
    std::map<int, int> map;

   public:
    // Constructors
    explicit DomainValue(const vec<int> &vals) : Domain(vals.size()) {
        for(int i = 0; i < vals.size(); i++) {
            values.push(vals[i]);
            map[vals[i]] = i;
        }
    }


    // Virtual Method conversion id to value
    int toIdv(int v) override { return map.find(v) == map.end() ? -1 : map[v]; }


    int toVal(int idv) override { return values[idv]; }
};
}   // namespace Cosoco
#endif