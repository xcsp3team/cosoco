#ifndef BINARYEXTENSION_H
#define BINARYEXTENSION_H

#include "constraints/extensions/Extension.h"

namespace Cosoco {
class BinaryExtension : public Extension {
   protected:
    Variable     *x, *y;   // Do not want to extends Binary
    int           nbtuples;
    vec<vec<int>> supportsForX;
    vec<vec<int>> supportsForY;

   public:
    // Constructors and initialisation
    BinaryExtension(Problem &p, std::string n, bool support, Variable *x, Variable *y);
    BinaryExtension(Problem &p, std::string n, bool support, Variable *x, Variable *y, BinaryExtension *hasSameTuples);

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