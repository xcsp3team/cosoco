#ifndef ELEMENT_H
#define ELEMENT_H

#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"

namespace Cosoco {

class Element : public GlobalConstraint {
   protected:
    Variable *index;
    int       posIndex;   // position of the index in the list (-1 if absent)
    bool      startAtOne;
    int       szVector;   // original size without index
   public:
    Element(Problem &p, std::string n, std::string t, int sz, Variable *i, bool one = false)
        : GlobalConstraint(p, n, t, sz), index(i), startAtOne(one) { }


    inline Variable *getVariableFor(int posx) { return scope[posx - (startAtOne ? 1 : 0)]; }


    bool isCorrectlyDefined() override {
        for(int i = 0; i < szVector; i++) {
            // if(scope[i] == index)
            //    throw std::logic_error("Constraint " + std::to_string(idc) + ": Element has index inside list");
            for(int j = i + 1; j < szVector; j++)
                if(scope[i] == scope[j])
                    throw std::logic_error("Constraint " + std::to_string(idc) +
                                           ": Element has contains the same variable twice inside list");
        }

        if(index->domain.minimum() != (startAtOne ? 1 : 0))
            return false;
        if(index->domain.maximum() != szVector - (startAtOne ? 0 : 1))
            return false;
        return true;
    }
};
}   // namespace Cosoco


#endif /* ELEMENT_H */
