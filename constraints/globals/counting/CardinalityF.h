#ifndef COSOCO_CARDINALITYF_H
#define COSOCO_CARDINALITYF_H
#include "constraints/globals/GlobalConstraint.h"


namespace Cosoco {


class AbstractCardinalityF : public GlobalConstraint {
   protected:
    int             offset;
    vec<int>        values2indexes;
    vec<Variable *> vars;
    vec<int>        values;
    vec<SparseSet>  possibles;
    vec<SparseSet>  mandatories;
    SparseSet       valueToCompute;

    virtual int doFiltering() = 0;   // 0 finish, 1 perform again, -1 fail

   public:
    // A weak cardinality filtering used when list is too big
    AbstractCardinalityF(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &v);


    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;
    void init(bool full);
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool isCorrectlyDefined() override;
};

class CardinalityF : public AbstractCardinalityF {
    vec<Variable *> occurs;

   public:
    CardinalityF(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &v, vec<Variable *> &o);
    int doFiltering() override;
};

class CardinalityInt : public AbstractCardinalityF {
    vec<int> occurs;

   public:
    CardinalityInt(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &v, vec<int> &o);
    int doFiltering() override;
};

}   // namespace Cosoco
#endif   // COSOCO_CARDINALITYF_H