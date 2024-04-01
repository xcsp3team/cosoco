//
// Created by audemard on 30/03/24.
//

#include <utility>

#include "CosocoCallbacks.h"

using namespace XCSP3Core;


void ManageIntension::intension(std::string id, Tree *tree) {
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
        FactoryConstraints::createConstraintUnary(callbacks.problem, id, x, values, true);
        return;
    }
    if(recognizePrimitives(std::move(id), tree))
        return;
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
    explicit PBinary1(CosocoCallbacks &m) : Primitive(m, "eq(x,y)") { pattern.root->type = OFAKEOP; }


    bool post() override {
        if(operators.size() != 1 || isRelationalOperator(operators[0]) == false)
            return false;
        if(createXopYk(callbacks.problem, operators[0], variables[0], variables[1], 0))
            return true;
        return false;
    }
};

class PBinary2 : public Primitive {   // x + 3 <op> y
   public:
    explicit PBinary2(CosocoCallbacks &m) : Primitive(m, "eq(add(x,3),y)") {
        pattern.root->type = OFAKEOP;   // We do not care between logical operator
    }


    bool post() override {
        if(operators.size() != 1 || isRelationalOperator(operators[0]) == false)
            return false;
        callbacks.buildConstraintPrimitive(id, expressionTypeToOrderType(operators[0]), callbacks.mappingXV[variables[0]],
                                           constants[0], callbacks.mappingXV[variables[1]]);

        return true;
    }
};


class PBinary3 : public Primitive {   // x = y <op> 3
   public:
    explicit PBinary3(CosocoCallbacks &m) : Primitive(m, "eq(y,add(x,3))") {
        pattern.root->type = OFAKEOP;   // We do not care between logical operator
    }


    bool post() override {
        if(operators.size() != 1 || isRelationalOperator(operators[0]) == false)
            return false;
        constants[0] = -constants[0];
        callbacks.buildConstraintPrimitive(id, expressionTypeToOrderType(operators[0]), callbacks.mappingXV[variables[0]],
                                           constants[0], callbacks.mappingXV[variables[1]]);

        return true;
    }
};


class PTernary1 : public Primitive {   // x = y <op> 3
   public:
    explicit PTernary1(CosocoCallbacks &m) : Primitive(m, "eq(add(y,z),x)") {
        pattern.root->type = OFAKEOP;   // We do not care between logical operator
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
}
