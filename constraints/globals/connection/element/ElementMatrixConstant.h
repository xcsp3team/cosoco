#ifndef COSOCO_ELEMENTMATRIXCONSTANT_H
#define COSOCO_ELEMENTMATRIXCONSTANT_H

#include "ElementMatrix.h"
#include "constraints/globals/GlobalConstraint.h"

namespace Cosoco {
class ElementMatrixConstant : public ElementMatrix {
   protected:
    vec<int> rsentinels;
    vec<int> csentinels;

   public:
    ElementMatrixConstant(Problem &p, std::string n, vec<vec<Variable *> > &m, Variable *ri, Variable *ci, int v);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};


}   // namespace Cosoco

#endif   // COSOCO_ELEMENTMATRIX_H
