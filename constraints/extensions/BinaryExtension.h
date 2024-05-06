#ifndef BINARYEXTENSION_H
#define BINARYEXTENSION_H

#include "constraints/extensions/Extension.h"

namespace Cosoco {
class BinaryExtension : public Extension {
   protected:
    vec<int>  resx;    // residue for x
    vec<int>  resy;    // residue for y
    Variable *x, *y;   // Do not want to extends Binary

    int           maxConflictsx, maxConflictsy;
    int           nbtuples;
    Matrix<bool> *matrix;

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

    void delayedConstruction(int id) override;

    size_t nbTuples() override;
};
}   // namespace Cosoco


#endif /* BINARYEXTENSION_H */
