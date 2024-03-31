//
// Created by audemard on 30/03/24.
//

#include "ManageIntension.h"

using namespace Cosoco;

//--------------------------------------------------------------------------------------
// Classes used to recognized expressions.
//--------------------------------------------------------------------------------------

class PrimitivePattern {
   public:
    Tree                       *canonized, pattern;
    std::vector<int>            constants;
    std::vector<std::string>    variables;
    std::vector<ExpressionType> operators;
    CosocoCallbacks            &manager;
    std::string                 id;


    PrimitivePattern(CosocoCallbacks &m, string expr) : pattern(expr), manager(m) { }


    virtual ~PrimitivePattern() { }


    PrimitivePattern *setTarget(std::string _id, Tree *c) {
        id        = _id;
        canonized = c;
        return this;
    }


    virtual bool post() = 0;


    bool match() {
        constants.clear();
        variables.clear();
        operators.clear();
        return Node::areSimilar(canonized->root, pattern.root, operators, constants, variables) && post();
    }
};


class PrimitiveUnary1 : public PrimitivePattern {   // x op k
   public:
    explicit PrimitiveUnary1(CosocoCallbacks &m) : PrimitivePattern(m, "eq(x,3)") { pattern.root->type = OFAKEOP; }


    bool post() override {
        if(operators.size() != 1 || isRelationalOperator(operators[0]) == false)
            return false;
        if(operators[0] == OEQ || operators[0] == ONE) {
            std::vector<int> values;
            values.push_back(constants[0]);
            manager.buildConstraintExtension(id, (XVariable *)manager.mapping[variables[0]], values, operators[0] == OEQ, false);
            return true;
        }
        if(operators[0] == OLE) {
            manager.buildConstraintPrimitive(id, LE, (XVariable *)manager.mapping[variables[0]], constants[0]);
            return true;
        }
        return false;
    }
};

class PrimitiveUnary2 : public PrimitivePattern {   // x op k
   public:
    explicit PrimitiveUnary2(CosocoCallbacks &m) : PrimitivePattern(m, "le(3,x)") { }


    bool post() override {
        manager.buildConstraintPrimitive(id, GE, (XVariable *)manager.mapping[variables[0]], constants[0]);
        return true;
    }
};

class PrimitiveUnary3 : public PrimitivePattern {   // x in {1,3 5...}
   public:
    explicit PrimitiveUnary3(CosocoCallbacks &m) : PrimitivePattern(m, "in(x,set(1,3,5))") { pattern.root->type = OFAKEOP; }


    bool post() override {
        if(operators.size() != 1 || (operators[0] != OIN && operators[0] != ONOTIN))
            return false;

        std::vector<int> values;
        for(Node *n : canonized->root->parameters[1]->parameters) values.push_back((dynamic_cast<NodeConstant *>(n))->val);
        if(values.size() == 0) {
            if(operators[0] == OIN)
                manager.buildConstraintFalse(id);
            else
                manager.buildConstraintTrue(id);
            return true;
        }
        manager.buildConstraintExtension(id, (XVariable *)manager.mapping[variables[0]], values, operators[0] == OIN, false);
        return true;
    }
};


class PrimitiveUnary4 : public PrimitivePattern {   // x>=1 and x<=4
   public:
    explicit PrimitiveUnary4(CosocoCallbacks &m) : PrimitivePattern(m, "and(le(x,1),le(4,x))") { pattern.root->type = OFAKEOP; }


    bool post() override {
        if(variables[0] != variables[1] || operators.size() != 1 || (operators[0] != OAND && operators[0] != OOR))
            return false;
        if(operators[0] == OAND) {
            if(constants[1] > constants[0])
                manager.buildConstraintFalse(id);
            else
                manager.buildConstraintPrimitive(id, (XVariable *)manager.mapping[variables[0]], true, constants[1],
                                                 constants[0]);
            return true;
        }
        if(constants[0] > constants[1])
            manager.buildConstraintTrue(id);
        else
            manager.buildConstraintPrimitive(id, (XVariable *)manager.mapping[variables[0]], false, constants[0] + 1,
                                             constants[1] - 1);
        return true;
    }
};


class PrimitiveBinary1 : public XCSP3Core::PrimitivePattern {   // x <op> y
   public:
    PrimitiveBinary1(CosocoCallbacks &m) : PrimitivePattern(m, "eq(x,y)") { pattern.root->type = OFAKEOP; }


    bool post() override {
        if(operators.size() != 1 || isRelationalOperator(operators[0]) == false)
            return false;
        manager.callback->buildConstraintPrimitive(id, expressionTypeToOrderType(operators[0]),
                                                   (XVariable *)manager.mapping[variables[0]], 0,
                                                   (XVariable *)manager.mapping[variables[1]]);
        return true;
    }
};

class PrimitiveBinary2 : public XCSP3Core::PrimitivePattern {   // x + 3 <op> y
   public:
    PrimitiveBinary2(CosocoCallbacks &m) : PrimitivePattern(m, "eq(add(x,3),y)") {
        pattern.root->type = OFAKEOP;   // We do not care between logical operator
    }


    bool post() override {
        if(operators.size() != 1 || isRelationalOperator(operators[0]) == false)
            return false;
        manager.callback->buildConstraintPrimitive(id, expressionTypeToOrderType(operators[0]),
                                                   (XVariable *)manager.mapping[variables[0]], constants[0],
                                                   (XVariable *)manager.mapping[variables[1]]);

        return true;
    }
};


class PrimitiveBinary3 : public XCSP3Core::PrimitivePattern {   // x = y <op> 3
   public:
    PrimitiveBinary3(CosocoCallbacks &m) : PrimitivePattern(m, "eq(y,add(x,3))") {
        pattern.root->type = OFAKEOP;   // We do not care between logical operator
    }


    bool post() override {
        if(operators.size() != 1 || isRelationalOperator(operators[0]) == false)
            return false;
        constants[0] = -constants[0];
        manager.callback->buildConstraintPrimitive(id, expressionTypeToOrderType(operators[0]),
                                                   (XVariable *)manager.mapping[variables[0]], constants[0],
                                                   (XVariable *)manager.mapping[variables[1]]);

        return true;
    }
};


class PrimitiveTernary1 : public XCSP3Core::PrimitivePattern {   // x = y <op> 3
   public:
    PrimitiveTernary1(CosocoCallbacks &m) : PrimitivePattern(m, "eq(add(y,z),x)") {
        pattern.root->type = OFAKEOP;   // We do not care between logical operator
    }


    bool post() override {
        if(operators.size() != 1 || isRelationalOperator(operators[0]) == false)
            return false;
        std::vector<XVariable *> list;
        for(string &s : variables) list.push_back((XVariable *)manager.mapping[s]);
        vector<int> coefs;
        coefs.push_back(1);
        coefs.push_back(1);
        coefs.push_back(-1);
        XCondition cond;
        cond.operandType = INTEGER;
        cond.op          = expressionTypeToOrderType(operators[0]);
        cond.val         = 0;
        manager.callback->buildConstraintSum(id, list, coefs, cond);

        return true;
    }
};


class PrimitiveTernary2 : public XCSP3Core::PrimitivePattern {   // x * y = z
   public:
    PrimitiveTernary2(CosocoCallbacks &m) : PrimitivePattern(m, "eq(mul(x,y),z)") { }


    bool post() override {
        manager.callback->buildConstraintMult(id, (XVariable *)manager.mapping[variables[0]],
                                              (XVariable *)manager.mapping[variables[1]],
                                              (XVariable *)manager.mapping[variables[2]]);
        return true;
    }
};


bool CosocoCallbacks::recognizePrimitives(std::string id, Tree *tree) {
    for(PrimitivePattern *p : patterns)
        if(p->setTarget(id, tree)->match())
            return true;
    return false;
}


void CosocoCallbacks::createPrimitivePatterns() {
    patterns.push_back(new PrimitiveUnary1(*this));
    patterns.push_back(new PrimitiveUnary2(*this));
    patterns.push_back(new PrimitiveUnary3(*this));
    patterns.push_back(new PrimitiveUnary4(*this));
    patterns.push_back(new PrimitiveBinary1(*this));
    patterns.push_back(new PrimitiveBinary2(*this));
    patterns.push_back(new PrimitiveBinary3(*this));
    patterns.push_back(new PrimitiveTernary1(*this));
    patterns.push_back(new PrimitiveTernary2(*this));
}

void ManageIntension::intension(CosocoCallbacks callbacks, Tree *tree) { }