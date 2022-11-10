#ifndef COSOCO_ELEMENTMATRIX_H
#define COSOCO_ELEMENTMATRIX_H

#include "constraints/globals/GlobalConstraint.h"

namespace Cosoco {
class ElementMatrix : public GlobalConstraint {
   public:
    ElementMatrix(Problem &p, std::string n, vec<vec<Variable *> > &m, Variable *ri, Variable *ci, int v);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;

   protected:
    vec<vec<Variable *> > matrix;
    Variable             *rindex;
    Variable             *cindex;
    int                   value;


    int rindexPosition;   // in scope
    int cindexPosition;   // in scope

    vec<int> rsentinels;
    vec<int> csentinels;
};
}   // namespace Cosoco

#endif   // COSOCO_ELEMENTMATRIX_H
