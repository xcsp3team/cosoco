#include "CosocoCallbacks.h"
using namespace Cosoco;

void CosocoCallbacks::beginInstance(InstanceType type) {
    problems.growTo(nbcores);
    decisionVariables.growTo(nbcores);

    for(int core = 0; core < nbcores; core++) problems[core] = type == CSP ? new Problem("") : new OptimizationProblem("");

    optimizationProblem         = type == COP;
    invertOptimization          = false;
    nbMDD                       = 0;
    insideGroup                 = false;
    auxiliaryIdx                = 0;
    nbIntension2Extention       = 0;
    nbSharedIntension2Extension = 0;
}

void CosocoCallbacks::endInstance() {
    if(auxiliaryIdx > 0)
        std::cout << "c " << auxiliaryIdx << " auxiliary variables\n";
    for(int core = 0; core < nbcores; core++) problems[core]->delayedConstruction();
    printf("c nb Intensions -> Extensions : %d (shared: %d)\n", nbIntension2Extention, nbSharedIntension2Extension);
}

void CosocoCallbacks::buildVariableInteger(string id, int minValue, int maxValue) {
    for(int core = 0; core < nbcores; core++) problems[core]->createVariable(id, *(new DomainRange(minValue, maxValue)));
}

void CosocoCallbacks::buildVariableInteger(string id, vector<int> &values) {
    for(int core = 0; core < nbcores; core++) problems[core]->createVariable(id, *(new DomainValue(vector2vec(values))));
}

void CosocoCallbacks::endVariables() { nbInitialsVariables = problems[0]->nbVariables(); }

void CosocoCallbacks::initGroups() {
    insideGroup = true;
    nbMDD       = 0;
    nbIntension = -1;
    i2eNumber   = -1;
}


void CosocoCallbacks::beginGroup(string name) { initGroups(); }


void CosocoCallbacks::endGroup() { insideGroup = false; }


void CosocoCallbacks::beginSlide(string id, bool circular) { initGroups(); }


void CosocoCallbacks::endSlide() {
    insideGroup = false;
    initGroups();
}


void CosocoCallbacks::beginBlock(string classes) { initGroups(); }


void CosocoCallbacks::endBlock() {
    initGroups();
    insideGroup = false;
}


void CosocoCallbacks::beginObjectives() { startToParseObjective = true; }


//--------------------------------------------------------------------------------------
// Basic constraints
//--------------------------------------------------------------------------------------


void CosocoCallbacks::buildConstraintExtension(string id, vector<XVariable *> list, vector<vector<int>> &origTuples, bool support,
                                               bool hasStar) {
    if(hasStar && !support)
        throw runtime_error("Extension constraint with star and conflict tuples is not yet supported");
    for(int core = 0; core < nbcores; core++) {
        toMyVariables(list, core);
        buildConstraintExtension2(id, vars, origTuples, support, hasStar, core);
    }
}


void CosocoCallbacks::buildConstraintExtension2(const string &id, vec<Variable *> &scope, const vector<vector<int>> &origTuples,
                                                bool support, bool hasStar, int core) const {
    vec<vec<int>> tuples;
    tuples.growTo(origTuples.size());

    for(unsigned int i = 0; i < origTuples.size(); i++) {
        tuples[i].growTo(scope.size());
        for(int j = 0; j < scope.size(); j++) {
            tuples[i][j] = origTuples[i][j] == STAR ? STAR : scope[j]->domain.toIdv(origTuples[i][j]);
        }
    }
    FactoryConstraints::createConstraintExtension(problems[core], id, scope, tuples, support, hasStar);
}


void CosocoCallbacks::buildConstraintExtension(string id, XVariable *variable, vector<int> &tuples, bool support, bool hasStar) {
    if(hasStar)
        throw runtime_error("Extension constraint with star is not yet supported");

    for(int core = 0; core < nbcores; core++)
        FactoryConstraints::createConstraintUnary(problems[core], id, problems[core]->mapping[variable->id], vector2vec(tuples),
                                                  support);
}

void CosocoCallbacks::buildConstraintExtensionAs(string id, vector<XVariable *> list, bool support, bool hasStar) {
    for(int core = 0; core < nbcores; core++)
        FactoryConstraints::createConstraintExtensionAs(problems[core], id, toMyVariables(list, core), support);
}


void CosocoCallbacks::buildConstraintIntension(string id, Tree *tree) {
    vec<Variable *> scope;
    for(string &s : tree->listOfVariables) scope.push(problems[0]->mapping[s]);

    // Check x = y1=k1 or y2=k2...
    bool match = true;
    if(tree->root->type == OEQ && tree->root->parameters[0]->type == OVAR && tree->root->parameters[1]->type == OOR) {
        for(Node *n : tree->root->parameters[1]->parameters)
            if(n->type != OEQ || n->parameters[0]->type != OVAR || n->parameters[1]->type != ODECIMAL) {
                match = false;
                break;
            }
        if(match) {
            for(int core = 0; core < nbcores; core++) {
                auto           *nv = dynamic_cast<NodeVariable *>(tree->root->parameters[0]);
                Variable       *r  = problems[core]->mapping[nv->var];
                vec<Variable *> cl;
                vec<int>        values;
                for(Node *n : tree->root->parameters[1]->parameters) {
                    auto *nv2 = dynamic_cast<NodeVariable *>(n->parameters[0]);
                    auto *nc2 = dynamic_cast<NodeConstant *>(n->parameters[1]);
                    cl.push(problems[core]->mapping[nv2->var]);
                    values.push(nc2->val);
                }
                FactoryConstraints::createConstraintXeqOrYeqK(problems[core], id, r, cl, values);
            }
            return;
        }
    }

    // Check |x - y| = z
    if(tree->root->type == OEQ && tree->root->parameters[0]->type == ODIST &&
       tree->root->parameters[0]->parameters[0]->type == OVAR && tree->root->parameters[0]->parameters[1]->type == OVAR &&
       tree->root->parameters[1]->type == OVAR) {
        for(int core = 0; core < nbcores; core++) {
            FactoryConstraints::createConstraintDistXYeqZ(problems[core], id, problems[core]->mapping[tree->listOfVariables[1]],
                                                          problems[core]->mapping[tree->listOfVariables[2]],
                                                          problems[core]->mapping[tree->listOfVariables[0]]);
        }
        return;
    }


    // Check x <= y + z
    if(tree->root->type == OLE && tree->root->parameters[1]->type == OADD && tree->root->parameters[0]->type == OVAR &&
       tree->root->parameters[1]->parameters.size() == 2 && tree->root->parameters[1]->parameters[0]->type == OVAR &&
       tree->root->parameters[1]->parameters[1]->type == OVAR) {
        vec<int> coeffs;
        coeffs.push(1);
        coeffs.push(1);
        coeffs.push(-1);
        for(int core = 0; core < nbcores; core++) {
            vec<Variable *> v;
            v.push(problems[core]->mapping[tree->listOfVariables[1]]);
            v.push(problems[core]->mapping[tree->listOfVariables[2]]);
            v.push(problems[core]->mapping[tree->listOfVariables[0]]);
            FactoryConstraints::createConstraintSum(problems[core], id, v, coeffs, 0, GE);   // x = y + k
        }
        return;
    }

    // check x + k = y
    if(tree->root->type == OEQ && tree->root->parameters[1]->type == OADD && tree->root->parameters[0]->type == OVAR &&
       tree->root->parameters[1]->parameters.size() == 2 && tree->root->parameters[1]->parameters[0]->type == OVAR &&
       tree->root->parameters[1]->parameters[1]->type == ODECIMAL) {
        auto *nc = dynamic_cast<NodeConstant *>(tree->root->parameters[1]->parameters[1]);
        int   k  = nc->val;
        for(int core = 0; core < nbcores; core++)
            FactoryConstraints::createConstraintXeqYplusk(problems[core], id, scope[0], scope[1], k);   // x = y + k
        return;
    }


    // check x + k = y
    if(tree->root->type == OEQ && tree->root->parameters[0]->type == OADD && tree->root->parameters[1]->type == OVAR &&
       tree->root->parameters[0]->parameters.size() == 2 && tree->root->parameters[0]->parameters[0]->type == OVAR &&
       tree->root->parameters[0]->parameters[1]->type == ODECIMAL) {
        auto *nc = dynamic_cast<NodeConstant *>(tree->root->parameters[0]->parameters[1]);
        int   k  = nc->val;
        for(int core = 0; core < nbcores; core++)
            FactoryConstraints::createConstraintXeqYplusk(problems[core], id, scope[1], scope[0], k);   // x = y + k
        return;
    }

    // check x - y = z
    if(tree->root->type == OEQ && tree->root->parameters[0]->type == OVAR && tree->root->parameters[1]->type == OSUB &&
       tree->root->parameters[1]->parameters[0]->type == OVAR && tree->root->parameters[1]->parameters[1]->type == OVAR &&
       scope.size() == 3) {
        for(int core = 0; core < nbcores; core++)
            FactoryConstraints::createConstraintDiff(problems[core], id, scope[0], scope[1], scope[2]);   // x = y - z
        return;
    }
    // Check x + y = z
    if(tree->root->type == OEQ && tree->root->parameters[0]->type == OVAR && tree->root->parameters[1]->type == OADD &&
       tree->root->parameters[1]->parameters.size() == 2 && tree->root->parameters[1]->parameters[0]->type == OVAR &&
       tree->root->parameters[1]->parameters[1]->type == OVAR && scope.size() == 3) {
        for(int core = 0; core < nbcores; core++)
            FactoryConstraints::createConstraintSum(problems[core], id, scope[1], scope[2], scope[0]);   // x+y = z
        return;
    }

    // Check x*y = z
    if(tree->root->type == OEQ && tree->root->parameters[0]->type == OVAR && tree->root->parameters[1]->type == OMUL &&
       tree->root->parameters[1]->parameters.size() == 2 && tree->root->parameters[1]->parameters[0]->type == OVAR &&
       tree->root->parameters[1]->parameters[1]->type == OVAR) {
        for(int core = 0; core < nbcores; core++) {
            Variable *x = problems[core]->mapping[tree->root->parameters[1]->parameters[0]->toString()];
            Variable *y = problems[core]->mapping[tree->root->parameters[1]->parameters[1]->toString()];
            Variable *z = problems[core]->mapping[tree->root->parameters[0]->toString()];
            FactoryConstraints::createConstraintMult(problems[core], id, x, y, z);
        }
        return;
    }

    // check x = (y=k)
    if(tree->root->type == OEQ && tree->root->parameters[0]->type == OVAR && tree->root->parameters[1]->type == OEQ &&
       tree->root->parameters[1]->parameters[0]->type == OVAR && tree->root->parameters[1]->parameters[1]->type == ODECIMAL) {
        auto *nc = dynamic_cast<NodeConstant *>(tree->root->parameters[1]->parameters[1]);
        int   k  = nc->val;
        for(int core = 0; core < nbcores; core++)
            FactoryConstraints::createConstraintXeqYeqK(problems[core], id, scope[0], scope[1], k);
        return;
    }

    nbIntension++;
    /*printf("c ");
    tree->prefixe();
    printf("\n");
*/

    if(tree->root->type == OEQ && tree->root->parameters[0]->type == OVAR) {
        Node *tmp                 = tree->root->parameters[0];
        tree->root->parameters[0] = tree->root->parameters[1];
        tree->root->parameters[1] = tmp;
    }


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

    // arguments can be null if block and constraint transformed (see ProgressivePArty and channel constraint)
    if(startToParseObjective == false && insideGroup && i2eNumber >= 0 && arguments() != nullptr &&
       (*arguments())[nbIntension].size() == (*arguments())[i2eNumber].size()) {
        // Test if everything match (domain and integer in the arguments...
        assert(nbIntension > 0);
        vector<vector<XVariable *>> args          = *arguments();
        bool                        sameArguments = true;
        for(unsigned int i = 0; i < args[nbIntension].size(); i++) {
            auto *xi1 = dynamic_cast<XInteger *>(args[nbIntension][i]);
            auto *xi2 = dynamic_cast<XInteger *>(args[i2eNumber][i]);
            if(xi1 != nullptr && xi2 != nullptr) {
                if(xi1->value != xi2->value) {
                    sameArguments = false;
                    break;
                }
            } else {
                if(xi1 == nullptr && xi2 == nullptr) {
                    if(args[nbIntension][i]->domain->equals(args[i2eNumber][i]->domain) == false) {
                        sameArguments = false;
                        break;
                    }
                } else {
                    sameArguments = false;
                    break;
                }
            }
        }
        if(problems[0]->constraints.last()->scope.size() != tree->listOfVariables.size())
            sameArguments = false;
        if(sameArguments == true) {   // And same domains -> create extension and leave
            auto *c = dynamic_cast<Extension *>(problems[0]->constraints.last());
            assert(c != nullptr);
            nbSharedIntension2Extension++;
            nbIntension2Extention++;
            for(int core = 0; core < nbcores; core++) {
                vec<Variable *> varsCore;
                for(string &s : tree->listOfVariables) varsCore.push(problems[core]->mapping[s]);
                FactoryConstraints::createConstraintExtensionAs(problems[core], id, varsCore, c->isSupport);
            }
            return;
        }
    }


    int nbTuples = 1;
    for(Variable *x : scope) nbTuples *= x->domain.maxSize();

    if(tree->root->type == OEQ && tree->root->parameters[1]->type == OVAR) {   // Easy to compute
        nbTuples = nbTuples / scope.last()->domain.maxSize();
    }


    if(startToParseObjective == false && intension2extensionLimit && nbTuples < intension2extensionLimit) {   // Create extension
        nbIntension2Extention++;
        std::vector<std::vector<int>> tuples;
        i2eNumber      = nbIntension;
        bool isSupport = false;
        Constraint::toExtensionConstraint(tree, scope, tuples, isSupport);

        for(int core = 0; core < nbcores; core++) {
            vec<Variable *> varsCore;
            for(Variable *tmp : scope) {
                varsCore.push(problems[core]->mapping[tmp->_name]);
            }
            buildConstraintExtension2(id, varsCore, tuples, isSupport, false, core);
        }
        return;
    }

    // Create intension
    for(int core = 0; core < nbcores; core++) {
        i2eNumber = -1;
        vars.clear();
        Tree *tmp = new Tree(tree->toString());
        for(string &s : tmp->listOfVariables) vars.push(problems[core]->mapping[s]);
        FactoryConstraints::createConstraintIntension(problems[core], id, tmp, vars);
    }
}

void CosocoCallbacks::buildConstraintPrimitive(string id, OrderType op, XVariable *x, int k, XVariable *y) {   // x + k op y
    assert(op == LE || op == LT || op == EQ || op == NE);


    if(k == 0 && (op == EQ || op == NE)) {
        for(int core = 0; core < nbcores; core++) {
            vars.clear();
            vars.push(problems[core]->mapping[x->id]);
            vars.push(problems[core]->mapping[y->id]);
            if(op == EQ)
                FactoryConstraints::createConstraintAllEqual(problems[core], id, vars);
            else
                FactoryConstraints::createConstraintAllDiff(problems[core], id, vars);
        }
        return;
    }
    if(op == LE || op == LT) {
        for(int core = 0; core < nbcores; core++) {
            vars.clear();
            vars.push(problems[core]->mapping[x->id]);
            vars.push(problems[core]->mapping[y->id]);
            FactoryConstraints::createConstraintLessThan(problems[core], id, vars[0], k, vars[1], op == LT);
        }
        return;
    }
    string t = k == 0 ? x->id : "add(" + x->id + "," + std::to_string(k) + ")";
    string tmp;
    if(op == EQ)
        tmp = "eq(";
    if(op == NE)
        tmp = "ne(";
    assert(op == EQ || op == NE);
    tmp = tmp + t + "," + y->id + ")";
    buildConstraintIntension(id, new Tree(tmp));   // TODO
}

void CosocoCallbacks::buildConstraintPrimitive(string id, OrderType op, XVariable *xx, int k) {   // x op k op is <= or >=
    for(int core = 0; core < nbcores; core++) {
        Variable *x = problems[core]->mapping[xx->id];
        vec<int>  values;
        for(int idv : x->domain) {
            int v = x->domain.toVal(idv);
            if((op == LE && v <= k) || (op == GE && v >= k))
                values.push(v);
        }
        FactoryConstraints::createConstraintUnary(problems[core], id, x, values, true);
    }
}

void CosocoCallbacks::buildConstraintPrimitive(string id, XVariable *xx, bool in, int min, int max) {   // x in/notin [min,max]
    for(int core = 0; core < nbcores; core++) {
        Variable *x = problems[core]->mapping[xx->id];
        vec<int>  values;
        for(int idv : x->domain) {
            int v = x->domain.toVal(idv);
            if(min <= v && v <= max)
                values.push(v);
        }
        FactoryConstraints::createConstraintUnary(problems[core], id, x, values, in);
    }
}

void CosocoCallbacks::buildConstraintMult(string id, XVariable *x, XVariable *y, XVariable *z) {
    if(x == y || x == z || y == z) {
        cout << "eq(" + z->id + ",mul(" + x->id + "," + y->id + "))\n";
        buildConstraintIntension(id, new Tree("eq(" + z->id + ",mul(" + x->id + "," + y->id + "))"));
        return;
    }
    for(int core = 0; core < nbcores; core++) {
        Variable *xx = problems[core]->mapping[x->id];
        Variable *yy = problems[core]->mapping[y->id];
        Variable *zz = problems[core]->mapping[z->id];
        FactoryConstraints::createConstraintMult(problems[core], id, xx, yy, zz);
    }
}

//--------------------------------------------------------------------------------------
// Language  constraints
//--------------------------------------------------------------------------------------

Cosoco::MDD *CosocoCallbacks::sameMDDAsPrevious(vec<Variable *> &list, int core) {
    if(insideGroup && nbMDD) {   // Check is the MDD is the same
        auto *ext = dynamic_cast<MDDExtension *>(problems[core]->constraints.last());
        assert(ext != nullptr);
        assert(list.size() == ext->scope.size());
        int same = true;
        for(int i = 0; i < list.size(); i++)
            if(list[i]->minimum() != ext->scope[i]->minimum() || list[i]->maximum() != ext->scope[i]->maximum() ||
               list[i]->domain.maxSize() != ext->scope[i]->domain.maxSize()) {
                same = false;
                break;
            }
        if(same)
            return ext->mdd;
    }
    return nullptr;
}


void CosocoCallbacks::buildConstraintMDD(string id, vector<XVariable *> &list, vector<XTransition> &transitions) {
    for(int core = 0; core < nbcores; core++) {
        toMyVariables(list, vars, core);
        Cosoco::MDD *mdd = nullptr;
        if((mdd = sameMDDAsPrevious(vars, core)) != nullptr) {
            FactoryConstraints::createConstraintMDD(problems[core], id, vars, mdd);
            continue;
        }
        vec<XTransition *> trans;
        for(auto &transition : transitions) trans.push(&transition);
        FactoryConstraints::createConstraintMDD(problems[core], id, vars, trans);
    }
    nbMDD++;
}


void CosocoCallbacks::buildConstraintRegular(string id, vector<XVariable *> &list, string start, vector<string> &final,
                                             vector<XTransition> &transitions) {
    for(int core = 0; core < nbcores; core++) {
        toMyVariables(list, vars, core);
        Cosoco::MDD *mdd = nullptr;
        if((mdd = sameMDDAsPrevious(vars, core)) != nullptr) {
            FactoryConstraints::createConstraintMDD(problems[core], id, vars, mdd);
            continue;
        }

        vec<XTransition *> trans;
        for(auto &transition : transitions) trans.push(&transition);
        FactoryConstraints::createConstraintRegular(problems[core], id, vars, start, final, trans);
    }
}

//--------------------------------------------------------------------------------------
// Graph constraints
//--------------------------------------------------------------------------------------

void CosocoCallbacks::buildConstraintCircuit(string id, vector<XVariable *> &list, int startIndex) {
    if(startIndex != 0)
        throw runtime_error("Circuit with startIndex != 0 is not yet supported");

    for(int core = 0; core < nbcores; core++) {
        toMyVariables(list, vars, core);

        for(Variable *x : vars) {
            auto *d = dynamic_cast<DomainRange *>(&(x->domain));
            if(d == nullptr)
                throw std::invalid_argument("Circuit constraint is not yet supported with special domain");
            if(x->domain.minimum() != 0)
                throw std::runtime_error("Circuit constraint is not yet supported with special domain");
            if(x->domain.maximum() != vars.size() - 1)
                throw std::runtime_error("Circuit constraint is not yet supported with special domain");
        }

        FactoryConstraints::createConstraintCircuit(problems[core], id, vars);
    }
}


//--------------------------------------------------------------------------------------
// Comparison constraints
//--------------------------------------------------------------------------------------

void CosocoCallbacks::buildConstraintAlldifferent(string id, vector<XVariable *> &list) {
    for(int core = 0; core < nbcores; core++)
        FactoryConstraints::createConstraintAllDiff(problems[core], id, toMyVariables(list, core));
}


void CosocoCallbacks::buildConstraintAlldifferentExcept(string id, vector<XVariable *> &list, vector<int> &except) {
    for(int core = 0; core < nbcores; core++)
        FactoryConstraints::createConstraintAllDiffExcept(problems[core], id, toMyVariables(list, core), except);
}


void CosocoCallbacks::buildConstraintAlldifferent(string id, vector<Tree *> &trees) {
    vector<string> auxiliaryVariables;
    insideGroup = false;
    createAuxiliaryVariablesAndExpressions(trees, auxiliaryVariables);
    // Create the new objective
    // core duplication is here
    for(int core = 0; core < nbcores; core++) {
        vars.clear();
        for(auto &auxiliaryVariable : auxiliaryVariables) vars.push(problems[core]->mapping[auxiliaryVariable]);
        FactoryConstraints::createConstraintAllDiff(problems[core], id, vars);
    }
    /*
    for(unsigned int i = 0; i < trees.size(); i++) {
        for(unsigned int j = i + 1; j < trees.size(); j++) {
            auto *n = new NodeNE();
            n->addParameter(trees[i]->root);
            n->addParameter(trees[j]->root);
            Tree *tmp = new Tree(n);
            // Problem with scope
            for(const string &s : trees[i]->listOfVariables) tmp->listOfVariables.push_back(s);
            for(const string &s : trees[j]->listOfVariables)
                if(std::find(tmp->listOfVariables.begin(), tmp->listOfVariables.end(), s) == tmp->listOfVariables.end())
                    tmp->listOfVariables.push_back(s);
            assert(tmp != nullptr);
            buildConstraintIntension(id, tmp);
        }
    }*/
}


void CosocoCallbacks::buildConstraintAlldifferentList(string id, vector<vector<XVariable *>> &origlists) {
    for(int core = 0; core < nbcores; core++) {
        vec<vec<Variable *>> lists;
        for(auto &origlist : origlists) {
            lists.push();
            toMyVariables(origlist, lists.last(), core);
        }
        FactoryConstraints::createConstraintAllDiffList(problems[core], id, lists);
    }
}


void CosocoCallbacks::buildConstraintAlldifferentMatrix(string id, vector<vector<XVariable *>> &matrix) {
    // lines
    for(auto &list : matrix) buildConstraintAlldifferent(id, list);

    // columns
    for(unsigned int i = 0; i < matrix[0].size(); i++) {
        vector<XVariable *> alldiff;
        for(unsigned int j = 0; j < matrix.size(); j++) alldiff.push_back(matrix[j][i]);
        buildConstraintAlldifferent(id, alldiff);
    }
}


void CosocoCallbacks::buildConstraintAllEqual(string id, vector<XVariable *> &list) {
    for(int core = 0; core < nbcores; core++)
        FactoryConstraints::createConstraintAllEqual(problems[core], id, toMyVariables(list, core));
}


void CosocoCallbacks::buildConstraintNotAllEqual(string id, vector<XVariable *> &list) {
    for(int core = 0; core < nbcores; core++)
        FactoryConstraints::createConstraintNotAllEqual(problems[core], id, toMyVariables(list, core));
}


void CosocoCallbacks::buildConstraintOrdered(string id, vector<XVariable *> &list, OrderType order) {
    vector<int> lengths;
    for(int core = 0; core < nbcores; core++)
        FactoryConstraints::createConstraintOrdered(problems[core], id, toMyVariables(list, core), lengths, order);
}


void CosocoCallbacks::buildConstraintOrdered(string id, vector<XVariable *> &list, vector<int> &lengths, OrderType order) {
    for(int core = 0; core < nbcores; core++)
        FactoryConstraints::createConstraintOrdered(problems[core], id, toMyVariables(list, core), lengths, order);
}


void CosocoCallbacks::buildConstraintLex(string id, vector<vector<XVariable *>> &lists, OrderType order) {
    for(int core = 0; core < nbcores; core++) {
        vec<Variable *> list1, list2;
        for(unsigned int i = 0; i < lists.size() - 1; i++) {
            toMyVariables(lists[i], list1, core);
            toMyVariables(lists[i + 1], list2, core);
            FactoryConstraints::createConstraintLex(problems[core], id, list1, list2, order);
        }
    }
}


void CosocoCallbacks::buildConstraintLexMatrix(string id, vector<vector<XVariable *>> &matrix, OrderType order) {
    // lines
    buildConstraintLex(id, matrix, order);

    // columns
    vector<vector<XVariable *>> tmatrix;
    for(unsigned int i = 0; i < matrix[0].size(); i++) {
        vector<XVariable *> tmp;
        for(unsigned int j = 0; j < matrix.size(); j++) tmp.push_back(matrix[j][i]);
        tmatrix.push_back(tmp);
    }
    buildConstraintLex(id, tmatrix, order);
}


//--------------------------------------------------------------------------------------
// Summing and counting constraints
//--------------------------------------------------------------------------------------

void CosocoCallbacks::buildConstraintSum(string id, vector<XVariable *> &list, XCondition &xc) {
    vector<int> coeffs;
    coeffs.assign(list.size(), 1);
    buildConstraintSum(id, list, coeffs, xc);
}


void CosocoCallbacks::buildConstraintSum(string id, vector<XVariable *> &list, vector<int> &origcoeffs, XCondition &xc) {
    for(int core = 0; core < nbcores; core++) {
        vars.clear();
        toMyVariables(list, vars, core);
        buildConstraintSum(id, vars, origcoeffs, xc, core);
    }
}


void CosocoCallbacks::buildConstraintSum(string id, vec<Variable *> &variables, vector<int> &coeffs, XCondition &xc, int core) {
    string xcvar        = xc.var;
    bool   varCondition = xc.operandType == VARIABLE;
    vector2vec(coeffs);
    if(varCondition) {
        xc.operandType = INTEGER;
        xc.val         = 0;
        variables.push(problems[core]->mapping[xcvar]);
        vals.push(-1);
    }
    if(xc.op != IN) {
        FactoryConstraints::createConstraintSum(problems[core], id, variables, vals, xc.val, xc.op);
    } else {
        // Intervals
        FactoryConstraints::createConstraintSum(problems[core], id, variables, vals, xc.min, GE);
        FactoryConstraints::createConstraintSum(problems[core], id, variables, vals, xc.max, LE);
    }
    if(varCondition) {
        xc.operandType = VARIABLE;
    }
}


void CosocoCallbacks::buildConstraintSum(string id, vector<XVariable *> &list, vector<XVariable *> &coeffs, XCondition &xc) {
    bool scalar = true;
    for(unsigned int i = 0; i < list.size(); i++) {
        if(list[i]->domain->minimum() != 0 || list[i]->domain->maximum() != 1) {
            scalar = false;
            break;
        }
        if(coeffs[i]->domain->minimum() != 0 || coeffs[i]->domain->maximum() != 1) {
            scalar = false;
            break;
        }
    }
    if(scalar && xc.operandType == INTEGER && xc.op == LE) {
        for(int core = 0; core < nbcores; core++) {
            vars.clear();
            vec<Variable *> c;
            toMyVariables(list, vars, core);
            toMyVariables(coeffs, c, core);
            FactoryConstraints::createConstraintSum(problems[core], id, vars, c, xc.val, LE);
        }
        return;
    }

    if(scalar && xc.operandType == VARIABLE && xc.op == LE) {
        for(int core = 0; core < nbcores; core++) {
            vars.clear();
            vec<Variable *> c;
            toMyVariables(list, vars, core);
            toMyVariables(coeffs, c, core);
            Variable *z = problems[core]->mapping[xc.var];
            FactoryConstraints::createConstraintSum(problems[core], id, vars, c, z, LE);
        }
        return;
    }
    vector<Tree *> trees;

    for(unsigned int i = 0; i < list.size(); i++) trees.push_back(new Tree("mul(" + list[i]->id + "," + coeffs[i]->id + ")"));

    buildConstraintSum(id, trees, xc);


    /*            string tmp = "add(";
                assert(list.size() == coeffs.size());
                for(unsigned int i = 0; i < list.size(); i++) {
                    tmp = tmp + "mul(" + list[i]->id + "," + coeffs[i]->id + ")";
                    if(i < list.size() - 1) tmp = tmp + ",";
                }
                if(xc.operandType == VARIABLE) {
                    xc.operandType = INTEGER;
                    xc.val = 0;
                    tmp = tmp + ",neg(" + xc.var + ")";
                }
                tmp = tmp + ")";
                if(xc.op != IN) {
                    if(xc.op == EQ) tmp = "eq(" + tmp;
                    if(xc.op == NE) tmp = "ne(" + tmp;
                    if(xc.op == LE) tmp = "le(" + tmp;
                    if(xc.op == LT) tmp = "lt(" + tmp;
                    if(xc.op == GE) tmp = "ge(" + tmp;
                    if(xc.op == GT) tmp = "gt(" + tmp;
                    tmp = tmp + "," + std::to_string(xc.val) + ")";
                    XCSP3Core::Tree *tree = new Tree(tmp);
                    buildConstraintIntension(id, tree);
                    return;
                }

                // Intervals
                buildConstraintIntension(id, new XCSP3Core::Tree("ge(" + tmp + "," + std::to_string(xc.min) + ")"));
                buildConstraintIntension(id, new XCSP3Core::Tree("le(" + tmp + "," + std::to_string(xc.max) + ")"));
                */
}


void CosocoCallbacks::buildConstraintSum(string id, vector<Tree *> &trees, XCondition &cond) {
    vector<int> coeffs;
    coeffs.assign(trees.size(), 1);
    buildConstraintSum(id, trees, coeffs, cond);
}


void CosocoCallbacks::buildConstraintSum(string id, vector<Tree *> &trees, vector<int> &coefs, XCondition &cond) {
    vector<string> auxiliaryVariables;
    insideGroup = false;
    createAuxiliaryVariablesAndExpressions(trees, auxiliaryVariables);
    // Create the new objective
    // core duplication is here
    for(int core = 0; core < nbcores; core++) {
        vars.clear();
        for(auto &auxiliaryVariable : auxiliaryVariables) vars.push(problems[core]->mapping[auxiliaryVariable]);

        buildConstraintSum(id, vars, coefs, cond, core);
    }
}


void CosocoCallbacks::buildConstraintAtMost(string id, vector<XVariable *> &list, int value, int k) {
    for(int core = 0; core < nbcores; core++)
        FactoryConstraints::createConstraintAtMost(problems[core], id, toMyVariables(list, core), value, k);
}


void CosocoCallbacks::buildConstraintAtLeast(string id, vector<XVariable *> &list, int value, int k) {
    for(int core = 0; core < nbcores; core++)
        FactoryConstraints::createConstraintAtLeast(problems[core], id, toMyVariables(list, core), value, k);
}


void CosocoCallbacks::buildConstraintExactlyK(string id, vector<XVariable *> &list, int value, int k) {
    for(int core = 0; core < nbcores; core++)
        FactoryConstraints::createConstraintExactly(problems[core], id, toMyVariables(list, core), value, k);
}


void CosocoCallbacks::buildConstraintExactlyVariable(string id, vector<XVariable *> &list, int value, XVariable *x) {
    for(int core = 0; core < nbcores; core++)
        FactoryConstraints::createConstraintExactlyVariable(problems[core], id, toMyVariables(list, core), value,
                                                            problems[core]->mapping[x->id]);
}


void CosocoCallbacks::buildConstraintNValues(string id, vector<XVariable *> &list, XCondition &xc) {
    if(!(xc.operandType == VARIABLE && xc.op == EQ))
        throw runtime_error("c Such nValues constraint Not yes supported");

    for(int core = 0; core < nbcores; core++)
        FactoryConstraints::createConstraintNValuesEQV(problems[core], id, toMyVariables(list, core),
                                                       problems[core]->mapping[xc.var]);
}


void CosocoCallbacks::buildConstraintCount(string id, vector<XVariable *> &list, vector<XVariable *> &values, XCondition &xc) {
    if(values.size() == 1) {
        vector<Tree *> trees;
        for(auto &xv : list) {
            string s = "eq(" + xv->id + "," + values[0]->id + ")";
            trees.push_back(new Tree(s));
        }
        buildConstraintSum(id, trees, xc);
        return;
    }
    throw runtime_error("c some count with variables values constraint is not yet supported ");
}


void CosocoCallbacks::buildConstraintCount(string id, vector<Tree *> &trees, vector<int> &values, XCondition &xc) {
    if(values.size() == 1) {
        vector<Tree *> newtrees;
        for(auto &tree : trees) {
            string s = "eq(" + tree->toString() + "," + to_string(values[0]) + ")";
            newtrees.push_back(new Tree(s));
        }
        buildConstraintSum(id, newtrees, xc);
        return;
    }
    throw runtime_error("c some count with trees and multiple values constraint is not yet supported ");
}


void CosocoCallbacks::buildConstraintCount(string id, vector<Tree *> &trees, vector<XVariable *> &values, XCondition &xc) {
    if(values.size() == 1) {
        vector<Tree *> newtrees;
        for(auto &tree : trees) {
            string s = "eq(" + tree->toString() + "," + values[0]->id + ")";
            newtrees.push_back(new Tree(s));
        }
        buildConstraintSum(id, newtrees, xc);
        return;
    }
    throw runtime_error("c some count with trees and multiple values variables constraint is not yet supported ");
}


void CosocoCallbacks::buildConstraintCardinality(string id, vector<XVariable *> &list, vector<int> values, vector<int> &intOccurs,
                                                 bool closed) {
    for(int core = 0; core < nbcores; core++) {
        vec<Occurs> occurs;
        for(int o : intOccurs) {
            occurs.push();
            occurs.last().value = o;
            occurs.last().type  = OCCURS_INTEGER;
        }
        FactoryConstraints::createConstraintCardinality(problems[core], id, toMyVariables(list, core), vector2vec(values),
                                                        occurs);
    }
}


void CosocoCallbacks::buildConstraintCardinality(string id, vector<XVariable *> &list, vector<int> values,
                                                 vector<XVariable *> &varOccurs, bool closed) {
    for(int core = 0; core < nbcores; core++) {
        vec<Occurs> occurs;
        for(XVariable *xv : varOccurs) {
            occurs.push();
            occurs.last().x    = problems[core]->mapping[xv->id];
            occurs.last().type = OCCURS_VARIABLE;
        }
        FactoryConstraints::createConstraintCardinality(problems[core], id, toMyVariables(list, core), vector2vec(values),
                                                        occurs);
    }
}


void CosocoCallbacks::buildConstraintCardinality(string id, vector<XVariable *> &list, vector<int> values,
                                                 vector<XInterval> &intervalOccurs, bool closed) {
    for(int core = 0; core < nbcores; core++) {
        vec<Occurs> occurs;
        for(XInterval &xi : intervalOccurs) {
            occurs.push();
            occurs.last().min  = xi.min;
            occurs.last().max  = xi.max;
            occurs.last().type = OCCURS_INTERVAL;
        }
        FactoryConstraints::createConstraintCardinality(problems[core], id, toMyVariables(list, core), vector2vec(values),
                                                        occurs);
    }
}


//--------------------------------------------------------------------------------------
// Connection constraints
//--------------------------------------------------------------------------------------

string CosocoCallbacks::createExpression(string minmax, OrderType op, vector<XVariable *> &list, string value) {
    string o;

    if(op == LE)
        o = "le";
    if(op == LT)
        o = "lt";
    if(op == GE)
        o = "ge";
    if(op == GT)
        o = "gt";
    if(op == EQ)
        o = "eq";
    if(op == NE)
        o = "ne";
    if(op == IN)
        throw runtime_error("not yet supported");

    string tmp = o + "(" + value + "," + minmax + "(";
    for(unsigned int i = 0; i < list.size(); i++) tmp = tmp + (i != 0 ? "," : "") + list[i]->id;
    tmp = tmp + "))";
    return tmp;
}


void CosocoCallbacks::buildConstraintMaximum(string id, vector<Tree *> &list, XCondition &xc) {
    if(xc.operandType == VARIABLE) {
        if(xc.op == EQ) {
            vector<string> auxiliaryVariables;
            insideGroup = false;
            createAuxiliaryVariablesAndExpressions(list, auxiliaryVariables);
            for(int core = 0; core < nbcores; core++) {
                vars.clear();
                for(auto &auxiliaryVariable : auxiliaryVariables) vars.push(problems[core]->mapping[auxiliaryVariable]);
                FactoryConstraints::createConstraintMaximumVariableEQ(problems[core], id, vars, problems[core]->mapping[xc.var]);
            }
            return;
        }
    }
    throw runtime_error("Maximum over set is not yet supported");
}

void CosocoCallbacks::buildConstraintMinimum(string id, vector<Tree *> &list, XCondition &xc) {
    if(xc.operandType == VARIABLE) {
        if(xc.op == EQ) {
            vector<string> auxiliaryVariables;
            insideGroup = false;
            createAuxiliaryVariablesAndExpressions(list, auxiliaryVariables);
            for(int core = 0; core < nbcores; core++) {
                vars.clear();
                for(auto &auxiliaryVariable : auxiliaryVariables) vars.push(problems[core]->mapping[auxiliaryVariable]);
                FactoryConstraints::createConstraintMinimumVariableEQ(problems[core], id, vars, problems[core]->mapping[xc.var]);
            }
            return;
        }
    }
    throw runtime_error("minimum over set is not yet supported");
}

void CosocoCallbacks::buildConstraintMaximum(string id, vector<XVariable *> &list, XCondition &xc) {
    if(xc.operandType == VARIABLE) {
        if(xc.op == EQ)
            for(int core = 0; core < nbcores; core++) {
                toMyVariables(list, vars, core);
                FactoryConstraints::createConstraintMaximumVariableEQ(problems[core], id, vars, problems[core]->mapping[xc.var]);
            }
        else
            buildConstraintIntension(id, new Tree(createExpression("max", xc.op, list, xc.var)));
        return;
    }

    if(xc.operandType == INTEGER) {
        if(xc.op == LE || xc.op == LT) {
            for(int core = 0; core < nbcores; core++) {
                toMyVariables(list, vars, core);
                FactoryConstraints::createConstraintMaximumLE(problems[core], id, vars, xc.op == LE ? xc.val : xc.val - 1);
            }
            return;
        }
        if(xc.op == GE || xc.op == GT) {
            for(int core = 0; core < nbcores; core++) {
                toMyVariables(list, vars, core);
                FactoryConstraints::createConstraintMaximumGE(problems[core], id, vars, xc.op == GE ? xc.val : xc.val + 1);
            }
            return;
        }
        buildConstraintIntension(id, new XCSP3Core::Tree(createExpression("max", xc.op, list, std::to_string(xc.val))));
        return;
    }
    // Interval
    for(int core = 0; core < nbcores; core++) {
        toMyVariables(list, vars, core);
        FactoryConstraints::createConstraintMaximumLE(problems[core], id, vars, xc.min);
        FactoryConstraints::createConstraintMaximumGE(problems[core], id, vars, xc.max);
    }
}


void CosocoCallbacks::buildConstraintMinimum(string id, vector<XVariable *> &list, XCondition &xc) {
    if(xc.operandType == VARIABLE) {
        if(xc.op == EQ)
            for(int core = 0; core < nbcores; core++) {
                toMyVariables(list, vars, core);
                FactoryConstraints::createConstraintMinimumVariableEQ(problems[core], id, vars, problems[core]->mapping[xc.var]);
            }
        else
            buildConstraintIntension(id, new XCSP3Core::Tree(createExpression("min", xc.op, list, xc.var)));
        return;
    }

    if(xc.operandType == INTEGER) {
        if(xc.op == LE || xc.op == LT) {
            for(int core = 0; core < nbcores; core++) {
                toMyVariables(list, vars, core);
                FactoryConstraints::createConstraintMinimumLE(problems[core], id, vars, xc.op == LE ? xc.val : xc.val - 1);
            }
            return;
        }
        if(xc.op == GE || xc.op == GT) {
            for(int core = 0; core < nbcores; core++) {
                toMyVariables(list, vars, core);
                FactoryConstraints::createConstraintMinimumGE(problems[core], id, vars, xc.op == GE ? xc.val : xc.val + 1);
            }
            return;
        }
        buildConstraintIntension(id, new XCSP3Core::Tree(createExpression("min", xc.op, list, std::to_string(xc.val))));
        return;
    }
    /// Interval
    for(int core = 0; core < nbcores; core++) {
        toMyVariables(list, vars, core);
        FactoryConstraints::createConstraintMinimumLE(problems[core], id, vars, xc.min);
        FactoryConstraints::createConstraintMinimumGE(problems[core], id, vars, xc.max);
    }
}


void CosocoCallbacks::buildConstraintElement(string id, vector<XVariable *> &list, int startIndex, XVariable *index,
                                             RankType rank, int value) {
    for(int core = 0; core < nbcores; core++) {
        Variable *idx = problems[core]->mapping[index->id];
        FactoryConstraints::createConstraintElementConstant(problems[core], id, toMyVariables(list, core), idx, startIndex,
                                                            value);
    }
}


void CosocoCallbacks::buildConstraintElement(string id, vector<XVariable *> &list, int startIndex, XVariable *index,
                                             RankType rank, XVariable *value) {
    if(value->id == index->id) {
        for(XVariable *x : list)
            if(x == value)
                throw runtime_error("Element variable value in list not yet implemented");


        for(int core = 0; core < nbcores; core++) {
            vec<vec<int>> tuples;
            toMyVariables(list, core);
            vars.push(problems[core]->mapping[value->id]);
            for(int i = 0; i < vars.size(); i++) {
                if(vars[i]->containsValue(i)) {
                    tuples.push();
                    for(int j = 0; j < vars.size(); j++) tuples.last().push(STAR);
                    tuples.last()[i]     = i;
                    tuples.last().last() = i;
                }
            }
            FactoryConstraints::createConstraintExtension(problems[core], id, vars, tuples, true, true);
        }
        return;
    }

    for(int core = 0; core < nbcores; core++) {
        Variable *idx = problems[core]->mapping[index->id];
        Variable *val = problems[core]->mapping[value->id];
        assert(val != nullptr);
        FactoryConstraints::createConstraintElementVariable(problems[core], id, toMyVariables(list, core), idx, startIndex, val);
    }
}


void CosocoCallbacks::buildConstraintElement(string id, vector<int> &list, int startIndex, XVariable *index, RankType rank,
                                             XVariable *value) {
    for(int core = 0; core < nbcores; core++) {
        vec<Variable *> tmp;
        vec<vec<int>>   tuples;
        Variable       *idx = problems[core]->mapping[index->id];
        Variable       *val = problems[core]->mapping[value->id];
        tmp.push(idx);
        tmp.push(val);
        for(int idv1 : idx->domain) {
            int v = idx->domain.toVal(idv1) - startIndex;
            if(v >= 0 && v < ((int)list.size()) && val->containsValue(list[v])) {
                tuples.push();
                tuples.last().push(idx->domain.toIdv(v + startIndex));
                tuples.last().push(val->domain.toIdv(list[v]));
            }
        }
        FactoryConstraints::createConstraintExtension(problems[core], id, tmp, tuples, true, false);
    }
}

void CosocoCallbacks::buildConstraintElement(string id, vector<XVariable *> &list, XVariable *index, int startIndex,
                                             XCondition &xc) {
    if(xc.op == EQ) {
        if(xc.operandType == INTEGER) {
            buildConstraintElement(id, list, startIndex, index, ANY, xc.val);
            return;
        } else {
            if(xc.operandType == VARIABLE) {
                auto *x = new XVariable(xc.var, nullptr);
                buildConstraintElement(id, list, startIndex, index, ANY, x);
                return;
            } else {
                throw runtime_error("element with condition interval not yet implemented");
            }
        }

        return;
    }
    insideGroup = false;
    string exp;
    if(xc.op == INTERVAL)
        throw runtime_error("Element with condition and interval not yet  implemented");
    if(xc.operandType == INTEGER)
        throw runtime_error("Element with condition and integer not yet  implemented");

    Variable *v   = problems[0]->mapping[list[0]->id];
    int       min = v->minimum();
    int       max = v->maximum();
    for(unsigned int i = 1; i < list.size(); i++) {
        v   = problems[0]->mapping[list[i]->id];
        min = min > v->minimum() ? v->minimum() : min;
        max = max < v->maximum() ? v->maximum() : max;
    }
    assert(nbcores == 1);   // TODO

    string auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";
    buildVariableInteger(auxVar, min, max);
    string tmp = "";
    if(xc.op == LE)
        tmp = "le(";
    if(xc.op == LT)
        tmp = "lt(";
    if(xc.op == GE)
        tmp = "ge(";
    if(xc.op == GT)
        tmp = "gt(";
    if(xc.op == NE)
        tmp = "ne(";
    tmp     = tmp + auxVar + "," + xc.var + ")";
    auto *x = new XVariable(auxVar, nullptr);
    buildConstraintIntension(id, new Tree(tmp));
    buildConstraintElement(id, list, startIndex, index, ANY, x);
}

void CosocoCallbacks::buildConstraintElement(string id, vector<vector<int>> &matrix, int startRowIndex, XVariable *rowIndex,
                                             int startColIndex, XVariable *colIndex, XVariable *value) {
    if(startRowIndex != 0 || startColIndex != 0)
        throw runtime_error("Element int matrix with startRowIndex or StartColIndex !=0 not yet supported");


    for(int core = 0; core < nbcores; core++) {
        vec<vec<int>>   tuples;
        vec<Variable *> vars;
        Variable       *row = problems[core]->mapping[rowIndex->id];
        Variable       *col = problems[core]->mapping[colIndex->id];
        Variable       *val = problems[core]->mapping[value->id];
        vars.push(row);
        vars.push(col);
        vars.push(val);

        for(int idr : row->domain) {
            int vr = row->domain.toVal(idr);

            for(int idc : col->domain) {
                int vc = col->domain.toVal(idc);
                if(vr >= 0 && vr < ((int)matrix.size()) && vc >= 0 && vc < ((int)matrix[vr].size()) &&
                   val->containsValue(matrix[vr][vc])) {
                    tuples.push();
                    tuples.last().push(idr);
                    tuples.last().push(idc);
                    tuples.last().push(val->domain.toIdv(matrix[vr][vc]));
                }
            }
        }
        FactoryConstraints::createConstraintExtension(problems[core], id, vars, tuples, true, false);
    }
}


void CosocoCallbacks::buildConstraintElement(string id, vector<vector<XVariable *>> &matrix, int startRowIndex,
                                             XVariable *rowIndex, int startColIndex, XVariable *colIndex, int value) {
    if(rowIndex == colIndex) {
        if(matrix.size() != matrix[0].size())
            throw runtime_error("Problem in element matrix");
        vector<XVariable *> tmp;
        for(unsigned int i = 0; i < matrix.size(); i++) tmp.push_back(matrix[i][i]);

        return buildConstraintElement(id, tmp, startRowIndex, rowIndex, ANY, value);
    }

    for(int core = 0; core < nbcores; core++) {
        vec<vec<Variable *>> m2;


        for(unsigned int i = 0; i < matrix.size(); i++) {
            m2.push();
            for(unsigned int j = 0; j < matrix[i].size(); j++) {
                m2.last().push(problems[core]->mapping[matrix[i][j]->id]);
            }
        }
        FactoryConstraints::createConstraintElementMatrix(problems[core], id, m2, problems[core]->mapping[rowIndex->id],
                                                          problems[core]->mapping[colIndex->id], value);
    }
}


void CosocoCallbacks::buildConstraintChannel(string id, vector<XVariable *> &list1, int startIndex1, vector<XVariable *> &list2,
                                             int startIndex2) {
    for(int core = 0; core < nbcores; core++) {
        vec<Variable *> X;
        vec<Variable *> Y;
        toMyVariables(list1, X, core);
        toMyVariables(list2, Y, core);
        FactoryConstraints::createContraintChannelXY(problems[core], id, X, Y, startIndex1, startIndex2);
    }
}


void CosocoCallbacks::buildConstraintChannel(string id, vector<XVariable *> &list, int startIndex, XVariable *value) {
    //  return forall(range(list.length), i -> intension(iff(list[i], eq(value, i))));
    int i = 0;
    for(XVariable *x : list) {
        string tmp = "iff(" + x->id + "," + "eq(" + value->id + "," + std::to_string(i) + "))";
        i++;
        buildConstraintIntension(id, new XCSP3Core::Tree(tmp));
    }
}


//--------------------------------------------------------------------------------------
// packing and schedulling constraints
//--------------------------------------------------------------------------------------


void CosocoCallbacks::buildConstraintNoOverlap(string id, vector<XVariable *> &origins, vector<int> &lengths, bool zeroIgnored) {
    if(!zeroIgnored)
        throw runtime_error("Nooverlap with zeroIgnored not yet supported");

    for(int core = 0; core < nbcores; core++) {
        toMyVariables(origins, core);
        for(int i = 0; i < vars.size(); i++)
            for(int j = i + 1; j < vars.size(); j++)
                FactoryConstraints::createConstraintNoOverlap(problems[core], id, vars[i], vars[j], lengths[i], lengths[j]);
    }
}


void CosocoCallbacks::buildConstraintNoOverlap(string id, vector<vector<XVariable *>> &origins,
                                               vector<vector<XVariable *>> &lengths, bool zeroIgnored) {
    if(!zeroIgnored)
        throw runtime_error("K dim Nooverlap with zeroIgnored not yet supported");


    for(unsigned int i = 0; i < origins.size(); i++) {
        for(unsigned int j = i + 1; j < origins.size(); j++) {
            string le1 = "le(add(" + origins[i][0]->id + "," + lengths[i][0]->id + ")," + origins[j][0]->id + ")";
            string le2 = "le(add(" + origins[j][0]->id + "," + lengths[j][0]->id + ")," + origins[i][0]->id + ")";

            std::stringstream stream;
            string            le3 = "le(add(" + origins[i][1]->id + "," + lengths[i][1]->id + ")," + origins[j][1]->id + ")";
            string            le4 = "le(add(" + origins[j][1]->id + "," + lengths[j][1]->id + ")," + origins[i][1]->id + ")";
            stream << "or(" << le1 << "," << le2 << "," << le3 << "," << le4 << ")";
            string tmp = stream.str();
            // intension(or(le(add(x1, w1), x2), le(add(x2, w2), x1), le(add(y1, h1), y2), le(add(y2, h2), y1)));
            buildConstraintIntension(id, new XCSP3Core::Tree(tmp));
        }
    }
}


void CosocoCallbacks::buildConstraintNoOverlap(string id, vector<vector<XVariable *>> &origins, vector<vector<int>> &lengths,
                                               bool zeroIgnored) {
    if(!zeroIgnored)
        throw runtime_error("K dim Nooverlap with zeroIgnored not yet supported");
    assert(origins.size() == lengths.size() && origins[0].size() == 2);
    if(false) {
        for(int core = 0; core < nbcores; core++) {
            vars.clear();
            vec<int>        w, h;
            vec<Variable *> X, Y;
            w.growTo(lengths.size());
            h.growTo(lengths.size());
            for(unsigned int i = 0; i < lengths.size(); i++) {
                w[i] = lengths[i][0];
                h[i] = lengths[i][1];
            }
            for(unsigned int i = 0; i < origins.size(); i++) {
                X.push(problems[core]->mapping[origins[i][0]->id]);
                Y.push(problems[core]->mapping[origins[i][1]->id]);
            }
            FactoryConstraints::createConstraintNoOverlap(problems[core], id, X, w, Y, h);
        }
    }
    for(int core = 0; core < nbcores; core++) {
        for(unsigned int i = 0; i < origins.size(); i++) {
            for(unsigned int j = i + 1; j < origins.size(); j++) {
                Variable *x1 = problems[core]->mapping[origins[i][0]->id];
                Variable *x2 = problems[core]->mapping[origins[j][0]->id];
                Variable *y1 = problems[core]->mapping[origins[i][1]->id];
                Variable *y2 = problems[core]->mapping[origins[j][1]->id];
                FactoryConstraints::createConstraintDisjunctive2D(problems[core], id, x1, x2, y1, y2, lengths[i][0],
                                                                  lengths[j][0], lengths[i][1], lengths[j][1]);
            }
        }
        // Add redundant constraint
        vec<Variable *> ox, oy;
        vec<int>        lx, ly;
        for(unsigned int i = 0; i < origins.size(); i++) {
            ox.push(problems[core]->mapping[origins[i][0]->id]);
            lx.push(lengths[i][0]);
            oy.push(problems[core]->mapping[origins[i][1]->id]);
            ly.push(lengths[i][1]);
        }
        int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;
        for(unsigned int i = 0; i < origins.size(); i++) {
            minX = std::min(minX, ox[i]->minimum());
            minY = std::min(minY, oy[i]->minimum());
            maxX = std::max(maxX, ox[i]->maximum() + lx[i]);
            maxY = std::max(maxY, oy[i]->maximum() + ly[i]);
        }
        FactoryConstraints::createConstraintCumulative(problems[core], id, ox, lx, ly, maxY - minY);
        FactoryConstraints::createConstraintCumulative(problems[core], id, oy, ly, lx, maxX - minX);
    }
    return;


    for(unsigned int i = 0; i < origins.size(); i++) {
        for(unsigned int j = i + 1; j < origins.size(); j++) {
            string le1 = "le(add(" + origins[i][0]->id + "," + std::to_string(lengths[i][0]) + ")," + origins[j][0]->id + ")";
            string le2 = "le(add(" + origins[j][0]->id + "," + std::to_string(lengths[j][0]) + ")," + origins[i][0]->id + ")";

            string le3 = "le(add(" + origins[i][1]->id + "," + std::to_string(lengths[i][1]) + ")," + origins[j][1]->id + ")";
            string le4 = "le(add(" + origins[j][1]->id + "," + std::to_string(lengths[j][1]) + ")," + origins[i][1]->id + ")";

            string tmp = "or(" + le1 + "," + le2 + "," + le3 + "," + le4 + ")";
            // intension(or(le(add(x1, w1), x2), le(add(x2, w2), x1), le(add(y1, h1), y2), le(add(y2, h2), y1)));
            buildConstraintIntension(id, new XCSP3Core::Tree(tmp));
            // exit(1);
        }
    }
}


void CosocoCallbacks::buildConstraintCumulative(string id, vector<XVariable *> &origins, vector<int> &lengths,
                                                vector<int> &heights, XCondition &xc) {
    for(int core = 0; core < nbcores; core++) {
        vars.clear();
        vec<int> h, l;
        vector2vec(lengths);
        vals.copyTo(l);
        vector2vec(heights);
        vals.copyTo(h);

        toMyVariables(origins, vars, core);
        FactoryConstraints::createConstraintCumulative(problems[core], id, vars, l, h, xc.val);
    }
}

//--------------------------------------------------------------------------------------
// Instantiation constraint
//--------------------------------------------------------------------------------------

void CosocoCallbacks::buildConstraintInstantiation(string id, vector<XVariable *> &list, vector<int> &values) {
    for(int core = 0; core < nbcores; core++)
        for(unsigned int i = 0; i < list.size(); i++) {
            vec<int> value;
            if(values[i] == STAR)
                continue;
            value.push(values[i]);
            FactoryConstraints::createConstraintUnary(problems[core], id, problems[core]->mapping[list[i]->id], value, true);
        }
}


void CosocoCallbacks::buildConstraintPrecedence(string id, vector<XVariable *> &list, vector<int> values) {
    for(int core = 0; core < nbcores; core++) {
        toMyVariables(list, core);
        FactoryConstraints::createConstraintPrecedence(problems[core], id, vars, vector2vec(values));
    }
}

void CosocoCallbacks::buildConstraintKnapsack(string id, vector<XVariable *> &list, vector<int> &weights, vector<int> &profits,
                                              int limit, XCondition &xc) {
    for(int core = 0; core < nbcores; core++) {
        toMyVariables(list, core);
        FactoryConstraints::createConstraintSum(problems[core], id, vars, vector2vec(weights), limit, LE);
    }
    buildConstraintSum(id, list, profits, xc);
}

void CosocoCallbacks::buildConstraintKnapsack(string id, vector<XVariable *> &list, vector<int> &weights, vector<int> &profits,
                                              XVariable *limit, XCondition &xc) {
    weights.push_back(-1);
    for(int core = 0; core < nbcores; core++) {
        toMyVariables(list, core);
        vars.push(problems[core]->mapping[limit->id]);
        FactoryConstraints::createConstraintSum(problems[core], id, vars, vector2vec(weights), 0, LE);
    }
    buildConstraintSum(id, list, profits, xc);
}


//--------------------------------------------------------------------------------------
// Objectives
//--------------------------------------------------------------------------------------

void CosocoCallbacks::buildObjectiveMinimizeExpression(string expr) {
    string tmp = "le(" + expr + ",0)";
    buildConstraintIntension("objective", new XCSP3Core::Tree(tmp));

    for(int core = 0; core < nbcores; core++) {
        auto *po = static_cast<OptimizationProblem *>(problems[core]);
        po->type = OptimisationType::Minimize;
        auto *oc = dynamic_cast<ObjectiveConstraint *>(problems[core]->constraints.last());
        po->addObjectiveUB(oc, true);
    }
}


void CosocoCallbacks::buildObjectiveMaximizeExpression(string expr) {
    string tmp = "ge(" + expr + ",0)";   // Fake value
    buildConstraintIntension("objective", new XCSP3Core::Tree(tmp));
    for(int core = 0; core < nbcores; core++) {
        auto *po = static_cast<OptimizationProblem *>(problems[core]);
        po->type = OptimisationType::Maximize;
        auto *oc = dynamic_cast<ObjectiveConstraint *>(problems[core]->constraints.last());
        po->addObjectiveLB(oc, true);
    }
}

void CosocoCallbacks::buildObjectiveMinimizeVariable(XVariable *x) {
    for(int core = 0; core < nbcores; core++) {
        FactoryConstraints::createConstraintUnaryLE(problems[core], "", problems[core]->mapping[x->id], 0);
        auto *po = static_cast<OptimizationProblem *>(problems[core]);
        po->type = OptimisationType::Minimize;
        auto *oc = dynamic_cast<ObjectiveConstraint *>(problems[core]->constraints.last());
        po->addObjectiveUB(oc, true);
    }
}

void CosocoCallbacks::buildObjectiveMaximizeVariable(XVariable *x) {
    for(int core = 0; core < nbcores; core++) {
        FactoryConstraints::createConstraintUnaryGE(problems[core], "", problems[core]->mapping[x->id], 0);
        auto *po = static_cast<OptimizationProblem *>(problems[core]);
        po->type = OptimisationType::Maximize;
        auto *oc = dynamic_cast<ObjectiveConstraint *>(problems[core]->constraints.last());
        po->addObjectiveLB(oc, true);
    }
}


void CosocoCallbacks::buildObjectiveMinimize(ExpressionObjective type, vector<XVariable *> &list) {
    vector<int> coeffs;
    coeffs.assign(list.size(), 1);
    buildObjectiveMinimize(type, list, coeffs);
}


void CosocoCallbacks::buildObjectiveMaximize(ExpressionObjective type, vector<XVariable *> &list) {
    vector<int> coeffs;
    coeffs.assign(list.size(), 1);
    buildObjectiveMaximize(type, list, coeffs);
}


void CosocoCallbacks::buildObjectiveMinimize(ExpressionObjective type, vec<Variable *> &variables, vector<int> &origcoeffs,
                                             int core) {
    auto *po = static_cast<OptimizationProblem *>(problems[core]);
    po->type = OptimisationType::Minimize;

    switch(type) {
        case EXPRESSION_O:
            break;   // useless, want to discard warning
        case SUM_O: {
            // Change optim type because LE->GE in Sum
            po->type           = OptimisationType::Maximize;
            invertOptimization = true;

            FactoryConstraints::createConstraintSum(problems[core], "objective", variables, vector2vec(origcoeffs), INT_MAX, LE);
            auto *oc = dynamic_cast<ObjectiveConstraint *>(problems[core]->constraints.last());
            po->addObjectiveLB(oc, true);
            break;
        }
        case PRODUCT_O:
            throw runtime_error("Objective product not yet supported");
            break;
        case MINIMUM_O: {
            FactoryConstraints::createConstraintMinimumLE(problems[core], "objective", variables, INT_MAX);
            auto *oc = dynamic_cast<ObjectiveConstraint *>(problems[core]->constraints.last());
            po->addObjectiveUB(oc, true);
            break;
        }
        case MAXIMUM_O: {
            FactoryConstraints::createConstraintMaximumLE(problems[core], "objective", variables, INT_MAX);
            auto *oc = dynamic_cast<ObjectiveConstraint *>(problems[core]->constraints.last());
            po->addObjectiveUB(oc, true);
            break;
        }
        case NVALUES_O: {
            FactoryConstraints::createConstraintNValuesLE(problems[core], "objective", variables, variables.size() + 1);
            auto *oc = dynamic_cast<ObjectiveConstraint *>(problems[core]->constraints.last());
            po->addObjectiveUB(oc, true);
            break;
        }
        case LEX_O:
            throw runtime_error("Objective expression not yet supported");
            break;
    }
}


void CosocoCallbacks::buildObjectiveMinimize(ExpressionObjective type, vector<XVariable *> &list, vector<int> &origcoeffs) {
    for(int core = 0; core < nbcores; core++) {
        toMyVariables(list, vars, core);
        buildObjectiveMinimize(type, vars, origcoeffs, core);
    }
}


void CosocoCallbacks::buildObjectiveMaximize(ExpressionObjective type, vec<Variable *> &variables, vector<int> &origcoeffs,
                                             int core) {
    auto *po = static_cast<OptimizationProblem *>(problems[core]);
    po->type = OptimisationType::Maximize;

    switch(type) {
        case EXPRESSION_O:
            break;   // useless, want to discard warning
        case SUM_O: {
            FactoryConstraints::createConstraintSum(problems[core], "objective", variables, vector2vec(origcoeffs), INT_MIN, GE);
            break;
        }
        case PRODUCT_O:
            throw runtime_error("Objective product not yet supported");
            break;
        case MINIMUM_O:
            FactoryConstraints::createConstraintMinimumGE(problems[core], "objective", variables, INT_MIN);
            break;
        case MAXIMUM_O:
            FactoryConstraints::createConstraintMaximumGE(problems[core], "objective", variables, INT_MIN);
            break;
        case NVALUES_O:
            throw runtime_error("Objective expression not yet supported");
            break;
        case LEX_O:
            throw runtime_error("Objective expression not yet supported");
            break;
    }
    auto *oc = dynamic_cast<ObjectiveConstraint *>(problems[core]->constraints.last());
    po->addObjectiveLB(oc, true);
}


void CosocoCallbacks::buildObjectiveMaximize(ExpressionObjective type, vector<XVariable *> &list, vector<int> &origcoeffs) {
    for(int core = 0; core < nbcores; core++) {
        toMyVariables(list, vars, core);
        buildObjectiveMaximize(type, vars, origcoeffs, core);
    }
}


void CosocoCallbacks::createAuxiliaryVariablesAndExpressions(vector<Tree *> &trees, vector<string> &auxiliaryVariables) {
    set<int>                   values;
    std::map<std::string, int> tuple;

    for(Tree *tree : trees) {
        string predicate = tree->toString();
        if(expressionsToAuxiliaryVariables.find(predicate) !=
           expressionsToAuxiliaryVariables.end()) {   // The aux = expression already exists
            auxiliaryVariables.push_back(expressionsToAuxiliaryVariables[predicate]);
            continue;
        }

        if(problems[0]->mapping.find(predicate) == problems[0]->mapping.end()) {   // This is a real expression
            string auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";

            if(problems[0]->mapping.find(auxVar) != problems[0]->mapping.end())
                throw runtime_error("Problem during creation of aux vars");
            auxiliaryVariables.push_back(auxVar);
            expressionsToAuxiliaryVariables[predicate] = auxVar;

            Range r = possibleValuesForExpressionInRange(tree->root);
            buildVariableInteger(auxVar, r.min, r.max);
            // Create the constraint auxVar = tree[i]
            // idem core duplication is done in the variable
            string tmp = "eq(" + auxVar + "," + predicate + ")";
            buildConstraintIntension("auxConstraint__" + std::to_string(auxiliaryIdx), new Tree(tmp));

        } else {   // The expresison is just a variable
            auxiliaryVariables.push_back(predicate);
        }
    }
}


void CosocoCallbacks::buildObjectiveMinimize(ExpressionObjective type, vector<Tree *> &trees, vector<int> &coefs) {
    vector<string> auxiliaryVariables;

    createAuxiliaryVariablesAndExpressions(trees, auxiliaryVariables);
    // Create the new objective
    // core duplication is here
    for(int core = 0; core < nbcores; core++) {
        vars.clear();
        for(auto &auxiliaryVariable : auxiliaryVariables) vars.push(problems[core]->mapping[auxiliaryVariable]);
        buildObjectiveMinimize(type, vars, coefs, core);
    }
}


void CosocoCallbacks::buildObjectiveMaximize(ExpressionObjective type, vector<Tree *> &trees, vector<int> &coefs) {
    vector<string> auxiliaryVariables;

    createAuxiliaryVariablesAndExpressions(trees, auxiliaryVariables);
    // Create the new objective
    // core duplication is here
    for(int core = 0; core < nbcores; core++) {
        vars.clear();
        for(auto &auxiliaryVariable : auxiliaryVariables) vars.push(problems[core]->mapping[auxiliaryVariable]);
        buildObjectiveMaximize(type, vars, coefs, core);
    }
}


void CosocoCallbacks::buildObjectiveMinimize(ExpressionObjective type, vector<Tree *> &trees) {
    vector<int> coeffs;
    coeffs.assign(trees.size(), 1);
    buildObjectiveMinimize(type, trees, coeffs);
}


void CosocoCallbacks::buildObjectiveMaximize(ExpressionObjective type, vector<Tree *> &trees) {
    vector<int> coeffs;
    coeffs.assign(trees.size(), 1);
    buildObjectiveMaximize(type, trees, coeffs);
}


void CosocoCallbacks::buildAnnotationDecision(vector<XVariable *> &list) {
    for(int core = 0; core < nbcores; core++) toMyVariables(list, decisionVariables[core], core);
}