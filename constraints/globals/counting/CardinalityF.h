#ifndef COSOCO_CARDINALITYF_H
#define COSOCO_CARDINALITYF_H
#include "constraints/globals/GlobalConstraint.h"


namespace Cosoco {


class CardinalityF : public GlobalConstraint {
   protected:
    std::map<int, int> values2indexes;
    vec<Variable *>    vars;
    vec<Variable *>    occurs;
    vec<int>           values;
    vec<SparseSet>     possibles;
    vec<SparseSet>     mandatories;
    SparseSet          valueToCompute;

    int doFiltering();   // 0 finish, 1 perform again, -1 fail

   public:
    // A weak cardinality filtering used when list is too big
    CardinalityF(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &v, vec<Variable *> &o);


    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;
    void init();
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool isCorrectlyDefined() override;
};

}   // namespace Cosoco
#endif   // COSOCO_CARDINALITYWEAK_H