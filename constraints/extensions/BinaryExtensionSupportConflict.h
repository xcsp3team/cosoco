#ifndef BINARYEXTENSIONCONFLICT_H
#define BINARYEXTENSIONCONFLICT_H

#include <set>

#include "constraints/extensions/Extension.h"

namespace Cosoco {
class BinaryExtensionConflict : public Extension {
   protected:
    Variable      *x, *y;   // Do not want to extends Binary
    int            nbtuples;
    vec<SparseSet> supportsForX, supportsForY;
    std::set<int>  inConflictsX, inConflictsY;
    vec<int>       resx;   // residue for x
    vec<int>       resy;   // residue for y


   public:
    // Constructors and initialisation
    BinaryExtensionConflict(Problem &p, std::string n, Variable *x, Variable *y);
    BinaryExtensionConflict(Problem &p, std::string n, Variable *x, Variable *y, BinaryExtensionConflict *hasSameTuples);

    // filtering
    bool filter(Variable *x) override;

    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;

    void addTuple(vec<int> &tupleIdv) override;
    void addTuple(int idv1, int idv2);

    size_t nbTuples() override;
};
}   // namespace Cosoco


#endif /* BINARYEXTENSION_H */