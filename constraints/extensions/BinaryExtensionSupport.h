#ifndef BINARYEXTENSIONSUPPORT_H
#define BINARYEXTENSIONSUPPORT_H

#include "constraints/extensions/Extension.h"

namespace Cosoco {
class BinaryExtensionSupport : public Extension {
   protected:
    Variable     *x, *y;   // Do not want to extends Binary
    int           nbtuples;
    vec<vec<int>> supportsForX;
    vec<vec<int>> supportsForY;
    vec<int>      limitX, limitY;
    vec<int>      resx;   // residue for x
    vec<int>      resy;   // residue for y
    uint64_t      lastConflict;

    bool filterOn(Variable *x, Variable *y, vec<vec<int>> &supportForXX, vec<vec<int>> &supportForYY, vec<int> &resXX,
                  vec<int> &resYY);

   public:
    // Constructors and initialisation
    BinaryExtensionSupport(Problem &p, std::string n, bool support, Variable *x, Variable *y);
    BinaryExtensionSupport(Problem &p, std::string n, bool support, Variable *x, Variable *y,
                           BinaryExtensionSupport *hasSameTuples);

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