//
// Created by audemard on 29/01/2021.
//

#ifndef COSOCO_XEQORYK_H
#define COSOCO_XEQORYK_H

#include "Solver.h"
#include "constraints/globals/GlobalConstraint.h"
namespace Cosoco {

class BasicNode {
   public:
    virtual ~BasicNode() = default;
    Variable *x;
    BasicNode(Variable *xx) : x(xx) { }
    virtual int  value()                  = 0;
    virtual int  minimum()                = 0;
    virtual int  maximum()                = 0;
    virtual bool setFalse(Solver *solver) = 0;
    virtual bool setTrue(Solver *solver)  = 0;
    int          size() { return (minimum() == 0 && maximum() == 1) ? 2 : 1; }
};

class BasicNodeEq : public BasicNode {
   public:
    int v;
    BasicNodeEq(Variable *xx, int vv) : BasicNode(xx), v(vv) { }
    int  value() override { return x->value(); }
    int  minimum() override { return (x->size() == 1 && x->value() == v) ? 1 : 0; }
    int  maximum() override { return (x->containsValue(v) == false) ? 0 : 1; }
    bool setTrue(Solver *solver) override { return solver->assignToVal(x, v); }
    bool setFalse(Solver *solver) override { return solver->delVal(x, v); }
};

class BasicNodeVar : public BasicNode {
   public:
    int v;
    BasicNodeVar(Variable *xx) : BasicNode(xx) { }
    int  value() override { return x->value(); }
    int  minimum() override { return x->minimum(); }
    int  maximum() override { return x->maximum(); }
    bool setTrue(Solver *solver) override { return solver->assignToVal(x, 1); }
    bool setFalse(Solver *solver) override { return solver->assignToVal(x, 0); }
};

class BasicNodeNe : public BasicNode {
   public:
    int v;
    BasicNodeNe(Variable *xx, int vv) : BasicNode(xx), v(vv) { }
    int  value() override { return x->value(); }
    int  minimum() override { return x->containsValue(v) == false ? 1 : 0; }
    int  maximum() override { return (x->size() == 1 && x->value() == v) ? 0 : 1; }
    bool setTrue(Solver *solver) override { return solver->delVal(x, v); }
    bool setFalse(Solver *solver) override { return solver->assignToVal(x, v); }
};

class BasicNodeLe : public BasicNode {   // le(x,10)
   public:
    int v;
    BasicNodeLe(Variable *xx, int vv) : BasicNode(xx), v(vv) { }
    int  value() override { return x->value(); }
    int  minimum() override { return x->maximum() <= v; }
    int  maximum() override { return x->minimum() > v ? 0 : 1; }
    bool setTrue(Solver *solver) override { return solver->delValuesGreaterOrEqualThan(x, v + 1); }
    bool setFalse(Solver *solver) override { return solver->delValuesLowerOrEqualThan(x, v); }
};

class BasicNodeGe : public BasicNode {   // le(x,10)
   public:
    int v;
    BasicNodeGe(Variable *xx, int vv) : BasicNode(xx), v(vv) { }
    int  value() override { return x->value(); }
    int  minimum() override { return x->minimum() >= v; }
    int  maximum() override { return x->maximum() < v ? 0 : 1; }
    bool setTrue(Solver *solver) override { return solver->delValuesLowerOrEqualThan(x, v - 1); }
    bool setFalse(Solver *solver) override { return solver->delValuesGreaterOrEqualThan(x, v); }
};

class BasicNodeIn : public BasicNode {   // le(x,10)
   public:
    vec<int> elements;
    BasicNodeIn(Variable *xx, vec<int> &s) : BasicNode(xx) { s.copyTo(elements); }
    int value() override { return x->value(); }
    int minimum() override {
        for(int idv : x->domain)
            if(elements.contains(x->domain.toVal(idv)) == false)
                return 0;
        return 1;
    }
    int maximum() override {
        for(int v : elements)
            if(x->containsValue(v))
                return 1;
        return 0;
    }
    bool setTrue(Solver *solver) override {
        for(int idv : x->domain)
            if(elements.contains(x->domain.toVal(idv)) == false && solver->delIdv(x, idv) == false)
                return false;
        return true;
    }
    bool setFalse(Solver *solver) override {
        for(int v : elements)
            if(solver->delVal(x, v) == false)
                return false;
        return true;
    }
};

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

}   // namespace Cosoco


#endif   // COSOCO_XEQORYK_H
