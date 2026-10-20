//
// Created by audemard on 12/09/2025.
//

#ifndef COSOCO_ELEMENTMATRIX_H
#define COSOCO_ELEMENTMATRIX_H
#include "GlobalConstraint.h"

namespace Cosoco {
class ElementMatrix : public GlobalConstraint {
   public:
    ElementMatrix(Problem &p, vec<vec<Variable *> > &m, Variable *ri, Variable *ci);
    ElementMatrix(Problem &p);

   protected:
    vec<vec<Variable *> > matrix;
    Variable             *rindex;
    Variable             *cindex;
    int                   value;


    int rindexPosition;   // in scope
    int cindexPosition;   // in scope
};


}   // namespace Cosoco


#endif   // COSOCO_ELEMENTMATRIX_H
