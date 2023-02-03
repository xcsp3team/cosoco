#ifndef ELEMENTVARIABLE_H
#define ELEMENTVARIABLE_H

#include "Element.h"
#include "mtl/Vec.h"
namespace Cosoco {

class ElementVariable : public Element {
    Variable       *value;
    vec<Variable *> list;

    /**
     * For each idx of a value v in result's domain, we store the index i of a variable from vector such that v is in
     * dom(vector[i]).
     */
    vec<int> indexSentinels;

    /**
     * For each variable in vector, we store a (normalized) value that is both in its domain and in result's domain
     */
    vec<int> valueSentinels;

   public:
    ElementVariable(Problem &p, std::string n, vec<Variable *> &vars, Variable *i, Variable *r, bool one = false);


    bool isCorrectlyDefined() override;

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;

   protected:
    bool validIndex(int v);
    bool filterIndex();
    bool validValue(int v);
    bool filterValue();
};
}   // namespace Cosoco


#endif /* ELEMENTVARIABLE_H */
