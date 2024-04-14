//
// Created by audemard on 30/03/24.
//

#include <utility>

#include "CosocoCallbacks.h"
using namespace XCSP3Core;
// /data/csp/GolombRuler-11-a3.xml

void replace_all_occurrences(std::string &input, const std::string &replace_word, const std::string &replace_by) {
    size_t pos = input.find(replace_word);
    while(pos != std::string::npos) {
        input.replace(pos, replace_word.size(), replace_by);       // Replace the substring with the specified string
        pos = input.find(replace_word, pos + replace_by.size());   // Find the next occurrence of the substring
    }
}

void ManageIntension::intension(std::string id, Tree *tree) {
    bool            done = false;
    vec<Variable *> scope;
    while(done == false) {
        scope.clear();
        if(callbacks.startToParseObjective == false)
            tree->canonize();


        //----------------------------------------------------------------------------
        // Unary constraints
        if(tree->arity() == 1 && callbacks.startToParseObjective == false) {
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
                callbacks.buildConstraintFalse("false");
                return;
            }
            FactoryConstraints::createConstraintUnary(callbacks.problem, id, x, values, true);
            return;
        }

        //----------------------------------------------------------------------------
        // Nary constraints

        if(recognizePrimitives(std::move(id), tree))
            return;

        //----------------------------------------------------------------------------
        //


        for(string &s : tree->listOfVariables) scope.push(callbacks.problem->mapping[s]);

        // Help to compute extension
        if(tree->root->type == OEQ && tree->root->parameters[1]->type == OVAR) {   // Easy to compute
            auto *nv = dynamic_cast<NodeVariable *>(tree->root->parameters[1]);
            if(nodeContainsVar(tree->root->parameters[0], nv->var) == false) {
                int pos = -1;
                for(int i = 0; i < scope.size(); i++)
                    if(scope[i]->_name == nv->var)
                        pos = i;
                Variable *tmp2               = scope[pos];
                scope[pos]                   = scope.last();
                scope.last()                 = tmp2;
                string tmp3                  = tree->listOfVariables[pos];
                tree->listOfVariables[pos]   = tree->listOfVariables.back();
                tree->listOfVariables.back() = tmp3;

                assert(scope.last()->_name == nv->var);
            }
        }

        // Compute cartesian product
        /*unsigned long long nbTuples = 1;
        for(Variable *x : scope) nbTuples *= x->domain.maxSize();
        if(tree->root->type == OEQ && tree->root->parameters[1]->type == OVAR)   // Easy to compute
            nbTuples = nbTuples / scope.last()->domain.maxSize();

        // If the constraint is small enough -> intension
        std::cout << nbTuples << " " << scope.size() << std::endl;
        if(nbTuples < 1000) {
            FactoryConstraints::createConstraintIntension(callbacks.problem, id, tree, scope);
            return;
        }
         */

        if(toExtension(id, tree, scope)) {
            std::cout << "extension : " << tree->root->toString();
            for(auto s : tree->listOfVariables) std::cout << callbacks.problem->mapping[s]->domain.maxSize() << " ";
            std::cout << "\n";
            return;
        }


        //----------------------------------------------------------------------------
        // decompose
        if(decompose(id, tree) == false)
            done = true;
    }
    std::cout << "intension : " << tree->root->toString();

    for(auto s : tree->listOfVariables) std::cout << callbacks.problem->mapping[s]->domain.maxSize() << " ";

    std::cout << "\n";
    FactoryConstraints::createConstraintIntension(callbacks.problem, id, tree, scope);
}


bool ManageIntension::decompose(XCSP3Core::Node *node) {
    if(node->type == OVAR || node->type == ODECIMAL)
        return false;

    bool           modified = false;
    vector<string> auxiliaryVariables;
    vector<Tree *> trees;
    for(unsigned int i = 0; i < node->parameters.size(); i++) {
        if(node->parameters[i]->type == OVAR || node->parameters[i]->type == ODECIMAL) {
            continue;   // Nothing to do
        }
        modified = true;
        trees.push_back(new Tree(node->parameters[i]));
        callbacks.createAuxiliaryVariablesAndExpressions(trees, auxiliaryVariables);
        delete trees[0];
        trees.pop_back();
        node->parameters[i] = new NodeVariable(auxiliaryVariables.back());
    }
    return modified;
}


bool ManageIntension::decompose(std::string id, XCSP3Core::Tree *tree) {
    assert(tree->arity() > 1);
    bool modified = false;
    // std::cout << "ici " << tree->root->toString() << "\n";

    if(tree->root->type == OEQ && (tree->root->parameters[0]->type == OVAR || tree->root->parameters[1]->type == OVAR)) {
        if(tree->root->parameters[0]->type == OVAR)
            modified = decompose(tree->root->parameters[1]);
        else
            modified = decompose(tree->root->parameters[0]);
    } else
        modified = decompose(tree->root);
    if(modified) {
        tree->listOfVariables.clear();
        extractVariables(tree->root, tree->listOfVariables);
    }
    // std::cout << "la " << tree->root->toString() << "  " << modified << "\n";
    return modified;
}

void ManageIntension::extractVariables(XCSP3Core::Node *node, vector<std::string> &listOfVariables) {
    if(node->type == ODECIMAL)
        return;
    if(node->type == OVAR) {
        auto *nodeVariable = dynamic_cast<NodeVariable *>(node);
        if(find(listOfVariables.begin(), listOfVariables.end(), nodeVariable->var) == listOfVariables.end())
            listOfVariables.push_back(nodeVariable->var);
        return;
    }
    for(Node *n : node->parameters) extractVariables(n, listOfVariables);
}


bool ManageIntension::existInCacheExtension(string &expr, vec<Variable *> &scope) {
    // Is in cache ?
    if(cachedExtensions.find(expr) != cachedExtensions.end()) {
        Constraint *c = cachedExtensions[expr];
        for(int i = 0; i < c->scope.size(); i++)
            if(c->scope[i]->domain.equals(&(scope[i]->domain)) == false)
                return false;
    } else
        return false;
    return true;
}

bool ManageIntension::toExtension(std::string id, XCSP3Core::Tree *tree, vec<Variable *> &scope) {
    unsigned long long nbTuples = 1;

    // Compute cartesian product
    for(Variable *x : scope) nbTuples *= x->domain.maxSize();
    if(tree->root->type == OEQ && tree->root->parameters[1]->type == OVAR)   // Easy to compute
        nbTuples = nbTuples / scope.last()->domain.maxSize();


    if(callbacks.startToParseObjective || callbacks.intension2extensionLimit == 0 ||
       nbTuples >= callbacks.intension2extensionLimit)
        return false;

    // Create generic intension
    std::string expr   = tree->toString();
    char        letter = 'a';
    for(Variable *x : scope) {
        replace_all_occurrences(expr, x->_name, {letter});
        letter++;
    }


    if(existInCacheExtension(expr, scope)) {   // expression is in cache
        FactoryConstraints::createConstraintExtensionAs(callbacks.problem, id, scope, cachedExtensions[expr]);
        callbacks.nbSharedIntension2Extension++;
        return true;
    }


    // Create extension
    callbacks.nbIntension2Extention++;
    std::vector<std::vector<int>> tuples;
    bool                          isSupport = false;
    Constraint::toExtensionConstraint(tree, scope, tuples, isSupport);

    vec<Variable *> varsCore;
    for(Variable *tmp : scope) varsCore.push(callbacks.problem->mapping[tmp->_name]);
    callbacks.buildConstraintExtension2(id, varsCore, tuples, isSupport, false);
    cachedExtensions[expr] = callbacks.problem->constraints.last();

    return true;
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
    explicit PBinary1(CosocoCallbacks &m) : Primitive(m, "eq(x,y)", 2) { pattern->root->type = OFAKEOP; }


    bool post() override {
        if(operators[0] == OIFF)
            return createXopYk(callbacks.problem, OEQ, variables[0], variables[1], 0);
        if(operators.size() != 1 || isRelationalOperator(operators[0]) == false)
            return false;
        return createXopYk(callbacks.problem, operators[0], variables[0], variables[1], 0);
    }
};

class PBinary2 : public Primitive {   // x + 3 <op> y
   public:
    explicit PBinary2(CosocoCallbacks &m) : Primitive(m, "eq(add(x,3),y)", 2) {
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
    explicit PBinary3(CosocoCallbacks &m) : Primitive(m, "eq(y,add(x,3))", 2) {
        pattern->root->type = OFAKEOP;   // We do not care between logical operator
    }


    bool post() override {
        if(isRelationalOperator(operators[0]) == false)
            return false;
        return createXopYk(callbacks.problem, operators[0], variables[0], variables[1], constants[0]);

        return true;
    }
};
class PBinary4 : public Primitive {   // x + y <op> 3
   public:
    explicit PBinary4(CosocoCallbacks &m) : Primitive(m, "le(add(y[0], x[0]), 3)", 2) {
        pattern->root->type = OFAKEOP;   // We do not care between logical operator
    }


    bool post() override {
        if(isRelationalOperator(operators[0]) == false)
            return false;
        std::vector<XVariable *> list;
        for(string &s : variables) list.push_back(callbacks.mappingXV[s]);
        XCondition cond;
        cond.operandType = INTEGER;
        cond.op          = expressionTypeToOrderType(operators[0]);
        cond.val         = constants[0];
        callbacks.buildConstraintSum(id, list, cond);
        return true;
    }
};

class PBinary5 : public Primitive {   // 3 <op> x + y
   public:
    explicit PBinary5(CosocoCallbacks &m) : Primitive(m, "le(3,add(y[0], x[0]))", 2) {
        pattern->root->type = OFAKEOP;   // We do not care between logical operator
    }


    bool post() override {
        if(isRelationalOperator(operators[0]) == false)
            return false;
        std::vector<XVariable *> list;
        for(string &s : variables) list.push_back(callbacks.mappingXV[s]);
        XCondition cond;
        cond.operandType = INTEGER;
        cond.op          = expressionTypeToOrderType(operators[0]);
        cond.val         = -constants[0];
        vector<int> coefs;
        coefs.push_back(-1);
        coefs.push_back(-1);
        callbacks.buildConstraintSum(id, list, coefs, cond);
        return true;
    }
};

class PBinary6 : public Primitive {   // x=  (y = 3)
   public:
    explicit PBinary6(CosocoCallbacks &m) : Primitive(m, "eq(eq(y,3),x)", 2) { }


    bool post() override {
        FactoryConstraints::createConstraintXeqYeqK(callbacks.problem, id, callbacks.problem->mapping[variables[1]],
                                                    callbacks.problem->mapping[variables[0]], constants[0]);
        return true;
    }
};


class PBinary7 : public Primitive {   // x=  mul(y,y)
   public:
    explicit PBinary7(CosocoCallbacks &m) : Primitive(m, "eq(mul(y,y),x)", 2) { }


    bool post() override {
        if(variables[0] != variables[1])
            return false;


        vector<vector<int>> tuples;
        vec<Variable *>     scope;
        scope.push(callbacks.problem->mapping[variables[0]]);
        scope.push(callbacks.problem->mapping[variables[2]]);
        string expr = "eq(mul(a,a),b)";

        if(callbacks.manageIntension->existInCacheExtension(expr, scope)) {   // expression is in cache
            FactoryConstraints::createConstraintExtensionAs(callbacks.problem, id, scope,
                                                            callbacks.manageIntension->cachedExtensions[expr]);
            callbacks.nbSharedIntension2Extension++;
            return true;
        }
        for(int idv : scope[0]->domain) {
            int v = scope[0]->domain.toVal(idv);
            tuples.push_back({v, v * v});
        }
        callbacks.buildConstraintExtension2(id, scope, tuples, true, false);
        callbacks.manageIntension->cachedExtensions[expr] = callbacks.problem->constraints.last();
        return true;
    }
};

// x = (y=k)
class PTernary1 : public Primitive {   // x = y <op> 3
   public:
    explicit PTernary1(CosocoCallbacks &m) : Primitive(m, "eq(add(y,z),x)", 3) {
        pattern->root->type                = OFAKEOP;   // We do not care between logical operator
        pattern->root->parameters[0]->type = OFAKEOP;
    }


    bool post() override {
        if(isRelationalOperator(operators[0]) == false)
            return false;

        if(operators[1] == OADD) {
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

        if(operators[0] != OEQ)
            return false;

        if(operators[1] == ODIST) {
            FactoryConstraints::createConstraintDistXYeqZ(callbacks.problem, id, callbacks.problem->mapping[variables[0]],
                                                          callbacks.problem->mapping[variables[1]],
                                                          callbacks.problem->mapping[variables[2]]);
            return true;
        }
        if(operators[1] == OMUL) {   // something like x*x=y
            callbacks.buildConstraintMult(id, callbacks.mappingXV[variables[0]], callbacks.mappingXV[variables[1]],
                                          callbacks.mappingXV[variables[2]]);
            return true;
        }
        if(operators[1] == OLE) {
            FactoryConstraints::createReification(callbacks.problem, id, callbacks.problem->mapping[variables[2]],
                                                  callbacks.problem->mapping[variables[0]],
                                                  callbacks.problem->mapping[variables[1]], operators[1]);
            return true;
        }
        if(operators[1] == OLT) {
            FactoryConstraints::createReification(callbacks.problem, id, callbacks.problem->mapping[variables[2]],
                                                  callbacks.problem->mapping[variables[0]],
                                                  callbacks.problem->mapping[variables[1]], operators[1]);
            return true;
        }
        if(operators[1] == OEQ) {
            FactoryConstraints::createReification(callbacks.problem, id, callbacks.problem->mapping[variables[2]],
                                                  callbacks.problem->mapping[variables[0]],
                                                  callbacks.problem->mapping[variables[1]], operators[1]);
            return true;
        }
        if(operators[1] == ONE) {
            FactoryConstraints::createReification(callbacks.problem, id, callbacks.problem->mapping[variables[2]],
                                                  callbacks.problem->mapping[variables[0]],
                                                  callbacks.problem->mapping[variables[1]], operators[1]);
            return true;
        }
        return false;
    }
};


class PTernary2 : public Primitive {
   public:
    explicit PTernary2(CosocoCallbacks &m) : Primitive(m, "le(z,add(x,y))", 3) { pattern->root->type = OFAKEOP; }
    bool post() override {
        std::vector<XVariable *> list;
        for(string &s : variables) list.push_back(callbacks.mappingXV[s]);
        vector<int> coefs;
        coefs.push_back(1);
        coefs.push_back(-1);
        coefs.push_back(-1);
        XCondition cond;
        cond.operandType = INTEGER;
        cond.op          = expressionTypeToOrderType(operators[0]);
        cond.val         = 0;
        callbacks.buildConstraintSum(id, list, coefs, cond);
        return true;
    }
};

class PQuater1 : public Primitive {
   public:
    explicit PQuater1(CosocoCallbacks &m) : Primitive(m, "eq(if(x,y,z),w)", 4) { }
    bool post() override {
        vec<Variable *> vars;
        for(const auto &v : variables) vars.push(callbacks.problem->mapping[v]);
        vec<vec<int>> tuples;
        for(int idv : vars[2]->domain) {
            int val = vars[2]->domain.toVal(idv);
            if(vars[3]->containsValue(val)) {
                tuples.push();
                tuples.last().push(vars[0]->domain.toIdv(0));
                tuples.last().push(STAR);
                tuples.last().push(idv);
                tuples.last().push(vars[3]->domain.toIdv(val));
            }
        }
        for(int idv : vars[1]->domain) {
            int val = vars[1]->domain.toVal(idv);
            if(vars[3]->containsValue(val)) {
                tuples.push();
                tuples.last().push(vars[0]->domain.toIdv(1));
                tuples.last().push(idv);
                tuples.last().push(STAR);
                tuples.last().push(vars[3]->domain.toIdv(val));
            }
        }
        FactoryConstraints::createConstraintExtension(callbacks.problem, "", vars, tuples, true, true);
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

class PNary2 : public FakePrimitive {   // or(x1,x2,x3..)
   public:
    explicit PNary2(CosocoCallbacks &c) : FakePrimitive(c) { }
    bool post() override {
        if(canonized->root->type == OOR) {
            for(Node *n : canonized->root->parameters)
                if(n->type != OVAR)
                    return false;

            vec<Variable *> cl;
            for(Node *n : canonized->root->parameters) {
                auto *nv = dynamic_cast<NodeVariable *>(n);
                cl.push(callbacks.problem->mapping[nv->var]);
            }
            FactoryConstraints::createConstraintAtLeast(callbacks.problem, id, cl, 1, 1);
            return true;
        }
        return false;
    }
};

class PNary3 : public FakePrimitive {   // or(x1,x2,x3..)
   public:
    explicit PNary3(CosocoCallbacks &c) : FakePrimitive(c) { }
    bool post() override {
        if(canonized->root->type != OEQ || canonized->root->parameters[1]->type != OVAR)
            return false;
        if(canonized->root->parameters[0]->type != OMIN && canonized->root->parameters[0]->type != OMAX)
            return false;
        for(Node *n : canonized->root->parameters[0]->parameters)
            if(n->type != OVAR)
                return false;

        vec<Variable *> vars;
        for(Node *n : canonized->root->parameters[0]->parameters) {
            auto *nv = dynamic_cast<NodeVariable *>(n);
            vars.push(callbacks.problem->mapping[nv->var]);
        }
        Variable *value = callbacks.problem->mapping[(dynamic_cast<NodeVariable *>(canonized->root->parameters[1]))->var];
        if(canonized->root->parameters[0]->type == OMIN)
            FactoryConstraints::createConstraintMinimumVariableEQ(callbacks.problem, id, vars, value);
        if(canonized->root->parameters[0]->type == OMAX)
            FactoryConstraints::createConstraintMaximumVariableEQ(callbacks.problem, id, vars, value);
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
    patterns.push(new PBinary4(callbacks));
    patterns.push(new PBinary5(callbacks));
    patterns.push(new PBinary6(callbacks));
    patterns.push(new PBinary7(callbacks));
    patterns.push(new PTernary1(callbacks));
    patterns.push(new PTernary2(callbacks));
    patterns.push(new PQuater1(callbacks));
    patterns.push(new PNary1(callbacks));
    patterns.push(new PNary2(callbacks));
    patterns.push(new PNary3(callbacks));
}
