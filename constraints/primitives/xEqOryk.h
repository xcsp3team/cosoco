//
// Created by audemard on 29/01/2021.
//

#ifndef COSOCO_XEQORYK_H
#define COSOCO_XEQORYK_H

#include "BasicNodes.h"
#include "Solver.h"
#include "constraints/globals/GlobalConstraint.h"
namespace Cosoco {

class BasicNode;

// in(sk[50][2],set(1,2,4))
class xEqGenOr : public GlobalConstraint {
    Variable        *result;
    vec<BasicNode *> nodes;

   public:
    xEqGenOr(Problem &p, std::string n, Variable *r, vec<Variable *> &vars, vec<BasicNode *> &nn);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

class xEqGenAnd : public GlobalConstraint {
    Variable        *result;
    vec<BasicNode *> nodes;

   public:
    xEqGenAnd(Problem &p, std::string n, Variable *r, vec<Variable *> &vars, vec<BasicNode *> &nn);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

class GenOr : public GlobalConstraint {
    vec<BasicNode *> nodes;
    int              s1, s2;

    int findSentinel(int other);

   public:
    GenOr(Problem &p, std::string n, vec<Variable *> &vars, vec<BasicNode *> &nn);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

class Or : public GlobalConstraint {
    vec<Variable *> nodes;
    int             s1, s2;

    int findSentinel(int other);

   public:
    Or(Problem &p, std::string n, vec<Variable *> &vars);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

}   // namespace Cosoco


#endif   // COSOCO_XEQORYK_H
