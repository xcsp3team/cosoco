#ifndef ELEMENTCONSTANT_H
#define ELEMENTCONSTANT_H

#include "Element.h"
#include "mtl/Vec.h"
namespace Cosoco {

class ElementConstant : public Element {
    int result;
    int indexInList;

   public:
    ElementConstant(Problem &p, std::string n, vec<Variable *> &vars, Variable *i, int kk, bool one = false);

    // Filtering method, return false if a conflict occurs
    virtual bool filter(Variable *x) override;

    // Checking
    virtual bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif /* ELEMENT_H */
