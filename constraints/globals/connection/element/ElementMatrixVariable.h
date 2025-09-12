//
// Created by audemard on 12/09/2025.
//

#ifndef COSOCO_ELEMENTMATRIXVARIABLE_H
#define COSOCO_ELEMENTMATRIXVARIABLE_H


#include "ElementMatrix.h"
#include "constraints/globals/GlobalConstraint.h"

namespace Cosoco {
class ElementMatrixVariable : public ElementMatrix {
   protected:
    vec<int>  rindexColSentinels, rindexValSentinels;
    vec<int>  cindexRowSentinels, cindexValSentinels;
    vec<int>  valueRowSentinels, valueColSentinels;
    int       vPosition;
    Variable *value;

    bool validRowIndex(int i);
    bool validColIndex(int i);
    bool validValue(int i);
    bool filterIndex();
    bool filterValue();

   public:
    ElementMatrixVariable(Problem &p, std::string n, vec<vec<Variable *> > &m, Variable *ri, Variable *ci, Variable *v);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};


}   // namespace Cosoco


#endif   // COSOCO_ELEMENTMATRIXVARIABLE_H
