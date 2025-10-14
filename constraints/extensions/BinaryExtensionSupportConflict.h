#ifndef BINARYEXTENSIONCONFLICT_H
#define BINARYEXTENSIONCONFLICT_H

#include "constraints/extensions/Extension.h"

namespace Cosoco {
class BinaryExtensionConflict : public Extension {
   protected:
    Variable     *x, *y;   // Do not want to extends Binary
    int           nbtuples;
    vec<vec<int>> supportsForX, supportsForY;


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