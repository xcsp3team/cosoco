//
// Created by audemard on 21/01/2026.
//

#ifndef COSOCO_BASICNODES_H
#define COSOCO_BASICNODES_H
#include "core/Variable.h"
#include "solver/Solver.h"

namespace Cosoco {
class BasicNode {
   public:
    virtual ~BasicNode() = default;
    Variable *x;
    explicit BasicNode(Variable *xx) : x(xx) { }
    virtual int  value(int k) = 0;
    int          size() { return minimum() == maximum() ? 1 : 2; }
    virtual int  minimum()                = 0;
    virtual int  maximum()                = 0;
    virtual bool setFalse(Solver *solver) = 0;
    virtual bool setTrue(Solver *solver)  = 0;
};

class BasicNodeEq : public BasicNode {
   public:
    int v;
    BasicNodeEq(Variable *xx, int vv) : BasicNode(xx), v(vv) { }
    int  value(int k) override { return v == k; }
    int  minimum() override { return (x->size() == 1 && x->value() == v) ? 1 : 0; }
    int  maximum() override { return (x->containsValue(v) == false) ? 0 : 1; }
    bool setTrue(Solver *solver) override { return solver->assignToVal(x, v); }
    bool setFalse(Solver *solver) override { return solver->delVal(x, v); }
};

class BasicNodeVar : public BasicNode {
   public:
    explicit BasicNodeVar(Variable *xx) : BasicNode(xx) { }
    int  value(int k) override { return k; }
    int  minimum() override { return x->minimum(); }
    int  maximum() override { return x->maximum(); }
    bool setTrue(Solver *solver) override { return solver->assignToVal(x, 1); }
    bool setFalse(Solver *solver) override { return solver->assignToVal(x, 0); }
};

class BasicNodeNegVar : public BasicNode {
   public:
    explicit BasicNodeNegVar(Variable *xx) : BasicNode(xx) { }
    int  value(int k) override { return 1 - k; }
    int  minimum() override { return x->maximum(); }
    int  maximum() override { return x->minimum(); }
    bool setTrue(Solver *solver) override { return solver->assignToVal(x, 0); }
    bool setFalse(Solver *solver) override { return solver->assignToVal(x, 1); }
};

class BasicNodeNe : public BasicNode {
   public:
    int v;
    BasicNodeNe(Variable *xx, int vv) : BasicNode(xx), v(vv) { }
    int  value(int k) override { return k != v; }
    int  minimum() override { return x->containsValue(v) == false ? 1 : 0; }
    int  maximum() override { return (x->size() == 1 && x->value() == v) ? 0 : 1; }
    bool setTrue(Solver *solver) override { return solver->delVal(x, v); }
    bool setFalse(Solver *solver) override { return solver->assignToVal(x, v); }
};

class BasicNodeLe : public BasicNode {   // le(x,10)
   public:
    int v;
    BasicNodeLe(Variable *xx, int vv) : BasicNode(xx), v(vv) { }
    int  value(int k) override { return k <= v; }
    int  minimum() override { return x->maximum() <= v; }
    int  maximum() override { return x->minimum() > v ? 0 : 1; }
    bool setTrue(Solver *solver) override { return solver->delValuesGreaterOrEqualThan(x, v + 1); }
    bool setFalse(Solver *solver) override { return solver->delValuesLowerOrEqualThan(x, v); }
};

class BasicNodeGe : public BasicNode {   // ge(x,10)
   public:
    int v;
    BasicNodeGe(Variable *xx, int vv) : BasicNode(xx), v(vv) { }
    int  value(int k) override { return k >= v; }
    int  minimum() override { return x->minimum() >= v; }
    int  maximum() override { return x->maximum() < v ? 0 : 1; }
    bool setTrue(Solver *solver) override { return solver->delValuesLowerOrEqualThan(x, v - 1); }
    bool setFalse(Solver *solver) override { return solver->delValuesGreaterOrEqualThan(x, v); }
};

class BasicNodeIn : public BasicNode {   // le(x,10)
   public:
    vec<int> elements;
    BasicNodeIn(Variable *xx, vec<int> &s) : BasicNode(xx) { s.copyTo(elements); }
    int value(int k) override { return elements.contains(k); }
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

}   // namespace Cosoco
#endif   // COSOCO_BASICNODES_H
