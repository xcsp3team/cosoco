//
// Created by audemard on 30/03/24.
//

#include <utility>

#include "CosocoCallbacks.h"
using namespace XCSP3Core;

void replace_all_occurrences(std::string &input, const std::string &replace_word, const std::string &replace_by) {
    size_t pos = input.find(replace_word);
    while(pos != std::string::npos) {
        input.replace(pos, replace_word.size(), replace_by);       // Replace the substring with the specified string
        pos = input.find(replace_word, pos + replace_by.size());   // Find the next occurrence of the substring
    }
}

void getVariables(Node *n, std::set<string> &variables) {
    NodeVariable *nv = dynamic_cast<NodeVariable *>(n);
    if(nv != nullptr) {
        variables.insert(nv->var);
        return;
    }
    NodeConstant *nc = dynamic_cast<NodeConstant *>(n);
    if(nc != nullptr)
        return;
    for(Node *np : n->parameters) getVariables(np, variables);
}

Node *ManageIntension::simplify(Node *node) {
    if(node->type == OVAR || node->type == ODECIMAL)
        return node;
    auto *nary = dynamic_cast<NodeOperator *>(node);

    if(node->type == OMUL) {
        unsigned int i = 0;
        while(i < nary->parameters.size()) {
            auto *tmp = dynamic_cast<NodeConstant *>(nary->parameters[i]);
            if(tmp != nullptr && tmp->val == 0)
                return new NodeConstant(0);
            if(tmp != nullptr && tmp->val == 1) {
                nary->parameters[i] = nary->parameters.back();
                nary->parameters.pop_back();
            } else
                i++;
        }
        if(nary->parameters.size() == 1)
            return nary->parameters[0];
    }
    for(unsigned int i = 0; i < nary->parameters.size(); i++) nary->parameters[i] = simplify(nary->parameters[i]);
    return node;
}


void ManageIntension::intension(std::string id, Tree *tree) {
    bool            done = false;
    vec<Variable *> scope;

    //----------------------------------------------------------------------------
    // Simplify constraint
    auto       *nary       = dynamic_cast<NodeOperator *>(tree->root);
    std::string expression = tree->root->toString();
    if(nary != nullptr) {
        for(unsigned int i = 0; i < nary->parameters.size(); i++) nary->parameters[i] = simplify(nary->parameters[i]);
    }
    if(expression != tree->root->toString()) {
        std::string  newExpression = tree->root->toString();
        unsigned int i             = 0;
        while(i < tree->listOfVariables.size()) {
            if(newExpression.find(tree->listOfVariables[i]) == std::string::npos) {
                tree->listOfVariables[i] = tree->listOfVariables.back();
                tree->listOfVariables.pop_back();
            } else
                i++;
        }
    }
    while(done == false) {
        scope.clear();
        if(callbacks.startToParseObjective == false)
            tree->canonize();

        //----------------------------------------------------------------------------
        // Unary constraints
        if(tree->arity() == 1 && callbacks.startToParseObjective == false) {
            std::map<std::string, int> tuple;
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
        // Primitive recognition

        if(callbacks.startToParseObjective == false && recognizePrimitives(std::move(id), tree))
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

        //----------------------------------------------------------------------------
        // to Extension constraints
        //----------------------------------------------------------------------------

        //------------------------------------------------------------------------------
        // Keep binary or(tree1, tree2) where all vars are in tree1 and tree2
        //------------------------------------------------------------------------------
        bool forceExtension = true;
        /*std::set<string> set;
        if(tree->arity() == 2) {
            for(Node *n : tree->root->parameters) {
                getVariables(n, set);
                if(set.size() > 2) {
                    forceExtension = false;
                    break;
                }
            }
        } else
            forceExtension = false;
        */
        forceExtension = tree->arity() == 2;

        if(toExtension(id, tree, scope, forceExtension)) {
            /*std::cout << "TO extension : " << tree->root->toString();
            Extension *ext = dynamic_cast<Extension *>(callbacks.problem->constraints.last());
            std::cout << ext->isSupport << " " << ext->nbTuples() << std::endl;
            for(Variable *x : scope) std::cout << x->_name << "(" << x->domain.maxSize() << ")" << " ";
            std::cout << "\n\n\n";
      */
            return;
        }


        //----------------------------------------------------------------------------
        // decomposition
        //----------------------------------------------------------------------------
        if(decompose(id, tree) == false)
            done = true;
    }

    std::cout << "c keep intension : " << tree->root->toString();
    for(const auto &s : tree->listOfVariables) std::cout << callbacks.problem->mapping[s]->domain.maxSize() << " ";
    std::cout << " arity: " << tree->arity() << "\n";

    // This is the end... nothing else than an intension constraint
    FactoryConstraints::createConstraintIntension(callbacks.problem, id, tree, scope);
}

/********************************************************************************************************************/
bool ManageIntension::decompose(XCSP3Core::Node *node) {
    if(callbacks.startToParseObjective)
        return false;
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
    if(options::boolOptions["decompose"].value == false)
        return false;
    if(tree->arity() == 1)   // => objective see MultiAgen in XCSP22 for instance
        return false;
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
    return modified;
}

/********************************************************************************************************************/

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

bool ManageIntension::toExtension(std::string id, XCSP3Core::Tree *tree, vec<Variable *> &scope, bool forceExtension) {
    unsigned long long nbTuples = 1;

    // Compute cartesian product
    for(Variable *x : scope) nbTuples *= x->domain.maxSize();
    if(tree->root->type == OEQ && tree->root->parameters[1]->type == OVAR)   // Easy to compute
        nbTuples = nbTuples / scope.last()->domain.maxSize();

    bool isProduct = true;
    if(tree->root->type == OEQ && tree->root->parameters[0]->type == OMUL && tree->root->parameters[1]->type == OVAR) {
        for(Node *n : tree->root->parameters[0]->parameters) {
            if(n->type != OVAR) {
                isProduct = false;
                break;
            }
        }
    } else
        isProduct = false;


    if(callbacks.startToParseObjective || options::intOptions["i2e"].value == 0)
        return false;
    if(forceExtension == false && isProduct == false && nbTuples >= ((unsigned long long)options::intOptions["i2e"].value))
        return false;

    // Create generic intension
    std::string expr   = tree->toString();
    char        letter = 'a';
    for(Variable *x : scope) {
        replace_all_occurrences(expr, x->_name, {letter});
        letter++;
    }


    callbacks.nbIntension2Extention++;
    if(existInCacheExtension(expr, scope)) {   // expression is in cache
        FactoryConstraints::createConstraintExtensionAs(callbacks.problem, id, scope, cachedExtensions[expr]);
        return true;
    }


    // Create extension
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
        return createXopYk(callbacks.problem, operators[0], variables[0], variables[1],
                           operators[0] == OEQ ? constants[0] : -constants[0]);

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
    explicit PBinary6(CosocoCallbacks &m) : Primitive(m, "eq(eq(y,3),x)", 2) { pattern->root->parameters[0]->type = OFAKEOP; }


    bool post() override {
        if(operators[0] == XCSP3Core::OEQ) {
            FactoryConstraints::createConstraintXeqYeqK(callbacks.problem, id, callbacks.problem->mapping[variables[1]],
                                                        callbacks.problem->mapping[variables[0]], constants[0]);
            return true;
        }
        if(operators[0] == XCSP3Core::ONE) {
            FactoryConstraints::createConstraintXeqYneK(callbacks.problem, id, callbacks.problem->mapping[variables[1]],
                                                        callbacks.problem->mapping[variables[0]], constants[0]);
            return true;
        }
        return false;
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

            callbacks.nbIntension2Extention++;
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


class PBinary8 : public Primitive {   // x=  (y <= 3)
   public:
    explicit PBinary8(CosocoCallbacks &m) : Primitive(m, "eq(le(62,z),x)", 2) { }


    bool post() override {
        FactoryConstraints::createConstraintXeqKleY(callbacks.problem, id, callbacks.problem->mapping[variables[1]],
                                                    callbacks.problem->mapping[variables[0]], constants[0]);
        return true;
    }
};

class PBinary9 : public Primitive {   // x=  (3 <= z)
   public:
    explicit PBinary9(CosocoCallbacks &m) : Primitive(m, "eq(le(z,62),x)", 2) { }


    bool post() override {
        FactoryConstraints::createConstraintXeqYleK(callbacks.problem, id, callbacks.problem->mapping[variables[1]],
                                                    callbacks.problem->mapping[variables[0]], constants[0]);
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
                                           // do the job (see PNary1)
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

class PNary3 : public FakePrimitive {   // x = min(x1,x2,x3..)
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

class PNary4 : public FakePrimitive {   // eq(add(__av1__,x[0],110),__av0__)
   public:
    explicit PNary4(CosocoCallbacks &c) : FakePrimitive(c) { }
    bool post() override {
        if(isRelationalOperator(canonized->root->type) == false || canonized->root->parameters[0]->type != OADD)
            return false;
        if(canonized->root->parameters[1]->type != OVAR && canonized->root->parameters[1]->type != ODECIMAL)
            return false;
        int             sum = 0;
        vec<Variable *> vars;
        vec<int>        coefs;

        for(Node *n : canonized->root->parameters[0]->parameters)
            if(n->type == OVAR)
                vars.push(callbacks.problem->mapping[(dynamic_cast<NodeVariable *>(n))->var]);
            else if(n->type == ODECIMAL)
                sum += (dynamic_cast<NodeConstant *>(n))->val;
            else
                return false;

        bool sumBoolean = true;
        for(Variable *x : vars)
            if(x->domain.maxSize() != 2)
                sumBoolean = false;
        if(sumBoolean && canonized->root->type == OEQ && canonized->root->parameters[1]->type == ODECIMAL) {
            int k = (dynamic_cast<NodeConstant *>(canonized->root->parameters[1]))->val;
            FactoryConstraints::createConstraintSumBooleanEQ(callbacks.problem, id, vars, -sum + k);
            return true;
        }

        if(sumBoolean && canonized->root->type == OLE && canonized->root->parameters[1]->type == ODECIMAL) {
            int k = (dynamic_cast<NodeConstant *>(canonized->root->parameters[1]))->val;
            FactoryConstraints::createConstraintSumBooleanLE(callbacks.problem, id, vars, -sum + k);
            return true;
        }
        coefs.growTo(vars.size(), 1);
        int k = 0;
        if(canonized->root->parameters[1]->type == OVAR) {
            coefs.push(-1);
            vars.push(callbacks.problem->mapping[(dynamic_cast<NodeVariable *>(canonized->root->parameters[1]))->var]);
        } else {
            k = (dynamic_cast<NodeConstant *>(canonized->root->parameters[1]))->val;
        }
        FactoryConstraints::createConstraintSum(callbacks.problem, id, vars, coefs, -sum + k,
                                                expressionTypeToOrderType(canonized->root->type));
        return true;
    }
};

class PNary5 : public FakePrimitive {   // eq(and(__av1__,x[0],110),__av0__)
                                        // Disabled, seems not efficient
   public:
    explicit PNary5(CosocoCallbacks &c) : FakePrimitive(c) { }
    bool post() override {
        if(canonized->root->type != OEQ || canonized->root->parameters[0]->type != OAND ||
           canonized->root->parameters[1]->type != OVAR)
            return false;
        vec<Variable *> vars;

        for(Node *n : canonized->root->parameters[0]->parameters) {
            if(n->type != OVAR)
                return false;
            vars.push(callbacks.problem->mapping[(dynamic_cast<NodeVariable *>(n))->var]);
        }
        FactoryConstraints::createConstraintXeqAndY(
            callbacks.problem, id,
            callbacks.problem->mapping[(dynamic_cast<NodeVariable *>(canonized->root->parameters[1]))->var], vars);
        return true;
    }
};

/*
static bool isLogicallyInverse(Node *n1, Node *n2) {
    ExpressionType tp1 = n1->type;
    ExpressionType tp2 = n2->type;
    if((tp1 == OLT && tp2 == OLE) || (tp1 == OLE && tp2 == OLT) || (tp1 == OGT && tp2 == OGE) || (tp1 == OGE && tp2 == OGT)) {
    }
    if((tp1 == OEQ && tp2 == ONE) || (tp1 == ONE && tp2 == OEQ)) {
    }
    return false;
}


class PNary6 : public FakePrimitive {   // OR(AND(?, ?), AND(?, ?))
   public:
    explicit PNary6(CosocoCallbacks &c) : FakePrimitive(c) { }
    bool post() override {
        if(canonized->root->type != OOR || canonized->root->parameters[0]->type != OAND ||
           canonized->root->parameters[1]->type != OAND)
            return false;
        Node *s00 = canonized->root->parameters[0]->parameters[0];
        Node *s01 = canonized->root->parameters[0]->parameters[1];
        Node *s10 = canonized->root->parameters[1]->parameters[0];
        Node *s11 = canonized->root->parameters[1]->parameters[1];
        if(isRelationalOperator(s00->type) == false || isRelationalOperator(s01->type) == false ||
           isRelationalOperator(s10->type) == false || isRelationalOperator(s11->type) == false)
            return false;


        vec<Variable *> vars;


        return true;
    }
};
 */

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
    patterns.push(new PBinary8(callbacks));
    patterns.push(new PBinary9(callbacks));
    patterns.push(new PTernary1(callbacks));
    patterns.push(new PTernary2(callbacks));
    patterns.push(new PQuater1(callbacks));
    patterns.push(new PNary1(callbacks));
    patterns.push(new PNary2(callbacks));
    patterns.push(new PNary3(callbacks));
    patterns.push(new PNary4(callbacks));
    // patterns.push(new PNary5(callbacks));
}
