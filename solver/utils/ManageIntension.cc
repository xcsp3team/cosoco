//
// Created by audemard on 30/03/24.
//

#include <utility>

#include "CosocoCallbacks.h"

using namespace XCSP3Core;


void ManageIntension::intension(std::string id, Tree *tree) {
    tree->prefixe();
    std::cout << "\n";
    if(tree->arity() == 1) {
        std::map<std::string, int> tuple;
        vec<Variable *>            scope;
        Variable                  *x = callbacks.problem->mapping[tree->listOfVariables[0]];
        vec<int>                   values;
        for(int idv : x->domain) {
            tuple[x->_name] = x->domain.toVal(idv);
            if(tree->evaluate(tuple))
                values.push(x->domain.toVal(idv));
        }
        if(values.size() == 0) {
            callbacks.buildConstraintFalse();
            return;
        }
        FactoryConstraints::createConstraintUnary(callbacks.problem, id, x, values, true);
        return;
    }
    if(recognizePrimitives(std::move(id), tree))
        return;
    // bug ternary 1 : gt(add(y[0],y[1]),x[1])
    assert(false);
}


static OrderType expressionTypeToOrderType(ExpressionType e) {
    if(e == OLE)
        return LE;
    if(e == OLT)
        return LT;
    if(e == OGE)
        return GE;
    if(e == OGT)
        return GT;
    if(e == OEQ)
        return XCSP3Core::EQ;
    if(e == ONE)
        return NE;
    assert(false);
    return LE;
}


static bool createXopYk(Problem *problem, ExpressionType ope, std::string x, std::string y, int k) {
    vec<Variable *> vars;
    vars.push(problem->mapping[x]);
    vars.push(problem->mapping[y]);
    order op = expressionTypeToOrderType(ope);

    if(k == 0 && (op == XCSP3Core::EQ || op == NE)) {
        if(op == XCSP3Core::EQ)
            FactoryConstraints::createConstraintAllEqual(problem, "", vars);
        else
            FactoryConstraints::createConstraintAllDiff(problem, "", vars);
        return true;
    }
    if(op == LE || op == LT) {
        FactoryConstraints::createConstraintLessThan(problem, "", vars[0], k, vars[1], op == LT);
        return true;
    }
    if(op == XCSP3Core::EQ) {
        FactoryConstraints::createConstraintXeqYplusk(problem, "", vars[0], vars[1], k);
        return true;
    }
    return false;
}
//--------------------------------------------------------------------------------------
// Classes used to recognized expressions.
//--------------------------------------------------------------------------------------


class PBinary1 : public Primitive {   // x <op> y
   public:
    explicit PBinary1(CosocoCallbacks &m) : Primitive(m, "eq(x,y)") { pattern->root->type = OFAKEOP; }


    bool post() override {
        if(operators.size() != 1 || isRelationalOperator(operators[0]) == false)
            return false;
        return createXopYk(callbacks.problem, operators[0], variables[0], variables[1], 0);
    }
};

class PBinary2 : public Primitive {   // x + 3 <op> y
   public:
    explicit PBinary2(CosocoCallbacks &m) : Primitive(m, "eq(add(x,3),y)") {
        pattern->root->type = OFAKEOP;   // We do not care between logical operator
    }


    bool post() override {
        if(isRelationalOperator(operators[0]) == false)
            return false;
        return createXopYk(callbacks.problem, operators[0], variables[0], variables[1],
                           operators[0] == OEQ ? -constants[0] : constants[0]);
    }
};


class PBinary3 : public Primitive {   // x = y <op> 3
   public:
    explicit PBinary3(CosocoCallbacks &m) : Primitive(m, "eq(y,add(x,3))") {
        pattern->root->type = OFAKEOP;   // We do not care between logical operator
    }


    bool post() override {
        if(isRelationalOperator(operators[0]) == false)
            return false;
        return createXopYk(callbacks.problem, operators[0], variables[0], variables[1],
                           operators[0] == OEQ ? -constants[0] : constants[0]);

        return true;
    }
};


class PTernary1 : public Primitive {   // x = y <op> 3
   public:
    explicit PTernary1(CosocoCallbacks &m) : Primitive(m, "eq(add(y,z),x)") {
        pattern->root->type = OFAKEOP;   // We do not care between logical operator
    }


    bool post() override {
        if(operators.size() != 1 || isRelationalOperator(operators[0]) == false)
            return false;
        std::vector<XVariable *> list;
        for(string &s : variables) list.push_back(callbacks.mappingXV[s]);
        vector<int> coefs;
        coefs.push_back(1);
        coefs.push_back(1);
        coefs.push_back(-1);
        XCondition cond;
        cond.operandType = INTEGER;
        cond.op          = expressionTypeToOrderType(operators[0]);
        cond.val         = 0;
        callbacks.buildConstraintSum(id, list, coefs, cond);
        return true;
    }
};


class PTernary2 : public Primitive {   // x * y = z
   public:
    explicit PTernary2(CosocoCallbacks &m) : Primitive(m, "eq(mul(x,y),z)") { }


    bool post() override {
        callbacks.buildConstraintMult(id, callbacks.mappingXV[variables[0]], callbacks.mappingXV[variables[1]],
                                      callbacks.mappingXV[variables[2]]);
        return true;
    }
};

class PTernary3 : public Primitive {
   public:
    explicit PTernary3(CosocoCallbacks &m) : Primitive(m, "eq(dist(x,y),z)") { }
    bool post() override {
        FactoryConstraints::createConstraintDistXYeqZ(callbacks.problem, id, callbacks.problem->mapping[variables[0]],
                                                      callbacks.problem->mapping[variables[1]],
                                                      callbacks.problem->mapping[variables[2]]);
        return true;
    }
};

class FakePrimitive : public Primitive {   // Does not try to match a pattern tree. Just return true, the post function d
                                           // do the job (see PNary1
   public:
    explicit FakePrimitive(CosocoCallbacks &c) : Primitive(c) { }
    bool match() override { return post(); }
};

class PNary1 : public FakePrimitive {
   public:
    explicit PNary1(CosocoCallbacks &c) : FakePrimitive(c) { }
    bool post() override {
        if(canonized->root->type == OEQ && canonized->root->parameters[0]->type == OOR &&
           canonized->root->parameters[1]->type == OVAR) {
            for(Node *n : canonized->root->parameters[0]->parameters)
                if(n->type != OEQ || n->parameters[0]->type != OVAR || n->parameters[1]->type != ODECIMAL) {
                    return false;
                }
            auto           *nv = dynamic_cast<NodeVariable *>(canonized->root->parameters[1]);
            Variable       *r  = callbacks.problem->mapping[nv->var];
            vec<Variable *> cl;
            vec<int>        values;
            for(Node *n : canonized->root->parameters[0]->parameters) {
                auto *nv2 = dynamic_cast<NodeVariable *>(n->parameters[0]);
                auto *nc2 = dynamic_cast<NodeConstant *>(n->parameters[1]);
                cl.push(callbacks.problem->mapping[nv2->var]);
                values.push(nc2->val);
            }
            FactoryConstraints::createConstraintXeqOrYeqK(callbacks.problem, id, r, cl, values);
            return true;
        }
        return false;
    }
};


bool ManageIntension::recognizePrimitives(std::string id, Tree *tree) {
    for(Primitive *p : patterns)
        if(p->setTarget(id, tree)->match())
            return true;
    return false;
}

ManageIntension::ManageIntension(CosocoCallbacks &c) : callbacks(c) { createPrimitives(); }


void ManageIntension::createPrimitives() {
    patterns.push(new PBinary1(callbacks));
    patterns.push(new PBinary2(callbacks));
    patterns.push(new PBinary3(callbacks));
    patterns.push(new PTernary1(callbacks));
    patterns.push(new PTernary2(callbacks));
    patterns.push(new PTernary3(callbacks));
    patterns.push(new PNary1(callbacks));
}
