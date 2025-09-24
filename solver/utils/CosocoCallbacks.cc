#include "CosocoCallbacks.h"

using namespace Cosoco;

void CosocoCallbacks::beginInstance(InstanceType type) {
    problem                        = type == CSP ? new Problem("") : new OptimizationProblem("");
    optimizationProblem            = type == COP;
    invertOptimization             = false;
    nbMDD                          = 0;
    insideGroup                    = false;
    auxiliaryIdx                   = 0;
    nbIntension2Extention          = 0;
    inArray                        = false;
    recognizeSpecialIntensionCases = false;
}

void CosocoCallbacks::endInstance() {
    if(auxiliaryIdx > 0)
        std::cout << "c " << auxiliaryIdx << " auxiliary variables\n";
    problem->delayedConstruction();
    printf("c nb Intensions -> Extensions : %d\n", nbIntension2Extention);
}

void CosocoCallbacks::buildVariableInteger(string id, int minValue, int maxValue) {
    Variable *x =
        problem->createVariable(id, *(new DomainRange(minValue, maxValue)), inArray ? problem->variablesArray.size() - 1 : -1);
    if(inArray == 1) {
        int dim = (int)std::count(id.begin(), id.end(), '[');
        for(int i = 0; i < dim; i++) arrayName.append("[]");
        problem->arrayNames.push_back(arrayName);
    }
    if(inArray) {
        problem->variablesArray.last().push(x);
        inArray = 2;
    }
    mappingXV[id] = new XVariable(id, nullptr);
}

void CosocoCallbacks::buildVariableInteger(string id, vector<int> &values) {
    Variable *x =
        problem->createVariable(id, *(new DomainValue(vector2vec(values))), inArray ? problem->variablesArray.size() - 1 : -1);
    if(inArray == 1) {
        int dim = (int)std::count(id.begin(), id.end(), '[');
        for(int i = 0; i < dim; i++) arrayName += "[]";
        problem->arrayNames.push_back(arrayName);
    }
    if(inArray) {
        problem->variablesArray.last().push(x);
        inArray = 2;
    }
    mappingXV[id] = new XVariable(id, nullptr);
}

void CosocoCallbacks::beginVariableArray(string id) {
    problem->variablesArray.push();
    arrayName = id;
    inArray   = 1;
}

void CosocoCallbacks::endVariableArray() { inArray = 0; }

void CosocoCallbacks::endVariables() {
    nbInitialsVariables     = problem->nbVariables();
    problem->nbOriginalVars = problem->nbVariables();
}

void CosocoCallbacks::initGroups() {
    insideGroup = true;
    nbMDD       = 0;
    nbIntension = -1;
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
    toMyVariables(list);
    buildConstraintExtension2(id, vars, origTuples, support, hasStar);
}

void CosocoCallbacks::buildConstraintExtensionAs(string id, vector<XVariable *> list, bool support, bool hasStar) {
    FactoryConstraints::createConstraintExtensionAs(problem, id, toMyVariables(list), problem->constraints.last());
}

void CosocoCallbacks::buildConstraintExtension2(const string &id, vec<Variable *> &scope, const vector<vector<int>> &origTuples,
                                                bool support, bool hasStar) const {
    vec<vec<int>> tuples;
    tuples.growTo(origTuples.size());

    for(unsigned int i = 0; i < origTuples.size(); i++) {
        tuples[i].growTo(scope.size());
        for(int j = 0; j < scope.size(); j++) {
            tuples[i][j] = origTuples[i][j] == STAR ? STAR : scope[j]->domain.toIdv(origTuples[i][j]);
        }
    }
    FactoryConstraints::createConstraintExtension(problem, id, scope, tuples, support, hasStar);
}


void CosocoCallbacks::buildConstraintExtension(string id, XVariable *variable, vector<int> &tuples, bool support, bool hasStar) {
    if(hasStar)
        throw runtime_error("Extension constraint with star is not yet supported");

    FactoryConstraints::createConstraintUnary(problem, id, problem->mapping[variable->id], vector2vec(tuples), support);
}


void CosocoCallbacks::buildConstraintIntension(string id, Tree *tree) { manageIntension->intension(id, tree); }

void CosocoCallbacks::buildConstraintPrimitive(string id, OrderType op, XVariable *x, int k, XVariable *y) {   // x + k op y
    assert(op == LE || op == LT || op == EQ || op == NE);


    if(k == 0 && (op == EQ || op == NE)) {
        vars.clear();
        vars.push(problem->mapping[x->id]);
        vars.push(problem->mapping[y->id]);
        if(op == EQ)
            FactoryConstraints::createConstraintAllEqual(problem, id, vars);
        else
            FactoryConstraints::createConstraintAllDiff(problem, id, vars);
        return;
    }
    if(op == LE || op == LT) {
        vars.clear();
        vars.push(problem->mapping[x->id]);
        vars.push(problem->mapping[y->id]);
        FactoryConstraints::createConstraintLessThan(problem, id, vars[0], k, vars[1], op == LT);
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
    Variable *x = problem->mapping[xx->id];
    vec<int>  values;
    for(int idv : x->domain) {
        int v = x->domain.toVal(idv);
        if((op == LE && v <= k) || (op == GE && v >= k))
            values.push(v);
    }
    FactoryConstraints::createConstraintUnary(problem, id, x, values, true);
}

void CosocoCallbacks::buildConstraintPrimitive(string id, XVariable *xx, bool in, int min,
                                               int max) {   // x in/notin [min,max]
    Variable *x = problem->mapping[xx->id];
    vec<int>  values;
    for(int idv : x->domain) {
        int v = x->domain.toVal(idv);
        if(min <= v && v <= max)
            values.push(v);
    }
    FactoryConstraints::createConstraintUnary(problem, id, x, values, in);
}


void CosocoCallbacks::buildConstraintMult(string id, XVariable *x, XVariable *y, XVariable *z) {
    if(x == y || x == z || y == z) {
        cout << "eq(" + z->id + ",mul(" + x->id + "," + y->id + "))\n";
        buildConstraintIntension(id, new Tree("eq(" + z->id + ",mul(" + x->id + "," + y->id + "))"));
        return;
    }
    Variable *xx = problem->mapping[x->id];
    Variable *yy = problem->mapping[y->id];
    Variable *zz = problem->mapping[z->id];
    FactoryConstraints::createConstraintMult(problem, id, xx, yy, zz);
}

//--------------------------------------------------------------------------------------
// Language  constraints
//--------------------------------------------------------------------------------------

Cosoco::MDD *CosocoCallbacks::sameMDDAsPrevious(vec<Variable *> &list) {
    if(insideGroup && nbMDD) {   // Check is the MDD is the same
        auto *ext = dynamic_cast<MDDExtension *>(problem->constraints.last());
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
    toMyVariables(list, vars);
    Cosoco::MDD *mdd = nullptr;
    if((mdd = sameMDDAsPrevious(vars)) != nullptr)
        FactoryConstraints::createConstraintMDD(problem, id, vars, mdd);
    else {
        vec<XTransition *> trans;
        for(auto &transition : transitions) trans.push(&transition);
        FactoryConstraints::createConstraintMDD(problem, id, vars, trans);
    }
    nbMDD++;
}


void CosocoCallbacks::buildConstraintRegular(string id, vector<XVariable *> &list, string start, vector<string> &final,
                                             vector<XTransition> &transitions) {
    toMyVariables(list, vars);

    // Some variables can appear multiple times.... We replace by a new variable and an allequal
    for(Variable *x : vars) x->fake = 0;
    for(int i = 0; i < vars.size(); i++) {
        if(vars[i]->fake == 1) {
            string       auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";
            DomainRange *dr     = nullptr;
            if((dr = dynamic_cast<DomainRange *>(&vars[i]->domain)) != nullptr) {
                buildVariableInteger(auxVar, dr->minimum(), dr->maximum());
            } else {
                auto *dv = dynamic_cast<DomainValue *>(&vars[i]->domain);
                assert(dv != nullptr);
                vector<int> values;
                for(int idv : *dv) values.push_back(idv);
                buildVariableInteger(auxVar, values);
            }
            vec<Variable *> tmp;
            tmp.push(vars[i]);
            tmp.push(problem->variables.last());
            FactoryConstraints::createConstraintAllEqual(problem, id, tmp);
            vars[i] = problem->variables.last();
        }
        vars[i]->fake = 1;
    }

    Cosoco::MDD *mdd = nullptr;
    if((mdd = sameMDDAsPrevious(vars)) != nullptr)
        FactoryConstraints::createConstraintMDD(problem, id, vars, mdd);
    else {
        vec<XTransition *> trans;
        for(auto &transition : transitions) trans.push(&transition);
        FactoryConstraints::createConstraintRegular(problem, id, vars, start, final, trans);
    }
}

//--------------------------------------------------------------------------------------
// Graph constraints
//--------------------------------------------------------------------------------------

void CosocoCallbacks::buildConstraintCircuit(string id, vector<XVariable *> &list, int startIndex) {
    if(startIndex != 0)
        throw runtime_error("Circuit with startIndex != 0 is not yet supported");

    toMyVariables(list, vars);
    FactoryConstraints::createConstraintCircuit(problem, id, vars);
}


//--------------------------------------------------------------------------------------
// Comparison constraints
//--------------------------------------------------------------------------------------

void CosocoCallbacks::buildConstraintAlldifferent(string id, vector<XVariable *> &list) {
    FactoryConstraints::createConstraintAllDiff(problem, id, toMyVariables(list));
}


void CosocoCallbacks::buildConstraintAlldifferentExcept(string id, vector<XVariable *> &list, vector<int> &except) {
    FactoryConstraints::createConstraintAllDiffExcept(problem, id, toMyVariables(list), except);
}

void CosocoCallbacks::buildConstraintAlldifferent(string id, vector<Tree *> &trees) {
    vector<string> auxiliaryVariables;
    insideGroup = false;
    createAuxiliaryVariablesAndExpressions(trees, auxiliaryVariables);
    // Create the new objective
    // core duplication is here
    vars.clear();
    for(auto &auxiliaryVariable : auxiliaryVariables) vars.push(problem->mapping[auxiliaryVariable]);
    FactoryConstraints::createConstraintAllDiff(problem, id, vars);
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
    vec<vec<Variable *>> lists;
    for(auto &origlist : origlists) {
        lists.push();
        toMyVariables(origlist, lists.last());
    }
    FactoryConstraints::createConstraintAllDiffList(problem, id, lists);
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
    FactoryConstraints::createConstraintAllEqual(problem, id, toMyVariables(list));
}


void CosocoCallbacks::buildConstraintNotAllEqual(string id, vector<XVariable *> &list) {
    FactoryConstraints::createConstraintNotAllEqual(problem, id, toMyVariables(list));
}


void CosocoCallbacks::buildConstraintOrdered(string id, vector<XVariable *> &list, OrderType order) {
    vector<int> lengths;
    FactoryConstraints::createConstraintOrdered(problem, id, toMyVariables(list), lengths, order);
}


void CosocoCallbacks::buildConstraintOrdered(string id, vector<XVariable *> &list, vector<int> &lengths, OrderType order) {
    FactoryConstraints::createConstraintOrdered(problem, id, toMyVariables(list), lengths, order);
}

void CosocoCallbacks::buildConstraintOrdered(string id, vector<XVariable *> &list, vector<XVariable *> &lengths,
                                             OrderType order) {
    vec<Variable *> vars, _lengths;
    toMyVariables(list, vars);
    toMyVariables(lengths, _lengths);
    FactoryConstraints::createConstraintOrdered(problem, id, vars, _lengths, order);
}

void CosocoCallbacks::buildConstraintLex(string id, vector<vector<XVariable *>> &lists, OrderType order) {
    vec<Variable *> list1, list2;
    for(unsigned int i = 0; i < lists.size() - 1; i++) {
        if(dynamic_cast<XInteger *>(lists[i][0]) != nullptr) {
            list1.clear();
            for(XVariable *xv : lists[i]) {
                auto  *xi     = dynamic_cast<XInteger *>(xv);
                string auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";
                buildVariableInteger(auxVar, xi->value, xi->value);
                list1.push(problem->mapping[auxVar]);
            }
        } else
            toMyVariables(lists[i], list1);
        if(dynamic_cast<XInteger *>(lists[i + 1][0]) != nullptr) {
            list2.clear();
            for(XVariable *xv : lists[i + 1]) {
                auto  *xi     = dynamic_cast<XInteger *>(xv);
                string auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";
                buildVariableInteger(auxVar, xi->value, xi->value);
                list2.push(problem->mapping[auxVar]);
            }
        } else
            toMyVariables(lists[i + 1], list2);

        FactoryConstraints::createConstraintLex(problem, id, list1, list2, order);
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
    vars.clear();
    toMyVariables(list, vars);
    buildConstraintSum(id, vars, origcoeffs, xc);
}


void CosocoCallbacks::buildConstraintSum(string id, vec<Variable *> &variables, vector<int> &coeffs, XCondition &xc) {
    string xcvar        = xc.var;
    bool   varCondition = xc.operandType == VARIABLE;
    vector2vec(coeffs);
    vec<Variable *> tmpVars;

    variables.copyTo(tmpVars);
    if(varCondition) {
        xc.operandType = INTEGER;
        xc.val         = 0;
        tmpVars.push(problem->mapping[xcvar]);
        vals.push(-1);
    }
    if(xc.op != IN && xc.op != NOTIN) {
        FactoryConstraints::createConstraintSum(problem, id, tmpVars, vals, xc.val, xc.op);
        return;
    }

    if(xc.op == IN && xc.operandType == INTERVAL) {
        FactoryConstraints::createConstraintSum(problem, id, tmpVars, vals, xc.min, GE);
        FactoryConstraints::createConstraintSum(problem, id, tmpVars, vals, xc.max, LE);
        return;
    }

    if(xc.op == IN) {
        string auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";
        buildVariableInteger(auxVar, xc.set);
        vector2vec(coeffs);   // Be careful we change the vector vals :(
        vals.push(-1);
        tmpVars.push(problem->mapping[auxVar]);
        FactoryConstraints::createConstraintSum(problem, id, tmpVars, vals, 0, EQ);
        return;
    }

    std::vector<int> tmp;
    if(xc.operandType == INTERVAL) {
        for(int i = xc.min; i <= xc.max; i++) tmp.push_back(i);
    } else
        tmp.assign(xc.set.begin(), xc.set.end());

    for(int i : tmp) FactoryConstraints::createConstraintSum(problem, id, tmpVars, vals, i, NE);
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
        vars.clear();
        vec<Variable *> c;
        toMyVariables(list, vars);
        toMyVariables(coeffs, c);
        FactoryConstraints::createConstraintSum(problem, id, vars, c, xc.val, LE);
        return;
    }

    if(scalar && xc.operandType == VARIABLE && xc.op == LE) {
        vars.clear();
        vec<Variable *> c;
        toMyVariables(list, vars);
        toMyVariables(coeffs, c);
        Variable *z = problem->mapping[xc.var];
        FactoryConstraints::createConstraintSum(problem, id, vars, c, z, LE);
        return;
    }
    vector<Tree *> trees;

    for(unsigned int i = 0; i < list.size(); i++) trees.push_back(new Tree("mul(" + list[i]->id + "," + coeffs[i]->id + ")"));

    buildConstraintSum(id, trees, xc);
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
    vars.clear();
    for(auto &auxiliaryVariable : auxiliaryVariables) vars.push(problem->mapping[auxiliaryVariable]);
    buildConstraintSum(id, vars, coefs, cond);
}


void CosocoCallbacks::buildConstraintAtMost(string id, vector<XVariable *> &list, int value, int k) {
    FactoryConstraints::createConstraintAtMost(problem, id, toMyVariables(list), value, k);
}


void CosocoCallbacks::buildConstraintAtLeast(string id, vector<XVariable *> &list, int value, int k) {
    FactoryConstraints::createConstraintAtLeast(problem, id, toMyVariables(list), value, k);
}


void CosocoCallbacks::buildConstraintExactlyK(string id, vector<XVariable *> &list, int value, int k) {
    FactoryConstraints::createConstraintExactly(problem, id, toMyVariables(list), value, k);
}


void CosocoCallbacks::buildConstraintExactlyVariable(string id, vector<XVariable *> &list, int value, XVariable *x) {
    FactoryConstraints::createConstraintExactlyVariable(problem, id, toMyVariables(list), value, problem->mapping[x->id]);
}


void CosocoCallbacks::buildConstraintNValues(string id, vector<XVariable *> &list, XCondition &xc) {
    if(xc.operandType == VARIABLE && xc.op == EQ) {
        FactoryConstraints::createConstraintNValuesEQV(problem, id, toMyVariables(list), problem->mapping[xc.var]);
        return;
    }
    if(xc.operandType == VARIABLE)
        throw runtime_error("c Such nValues constraint Not yes supported");

    if(xc.operandType == INTEGER) {
        if(xc.op == LE) {
            FactoryConstraints::createConstraintNValuesLE(problem, id, toMyVariables(list), xc.val);
            return;
        }
        if(xc.op == LT) {
            FactoryConstraints::createConstraintNValuesLE(problem, id, toMyVariables(list), xc.val - 1);
            return;
        }
        if(xc.op == GE) {
            FactoryConstraints::createConstraintNValuesGE(problem, id, toMyVariables(list), xc.val);
            return;
        }
        if(xc.op == GT) {
            FactoryConstraints::createConstraintNValuesGE(problem, id, toMyVariables(list), xc.val + 1);
            return;
        }
        if(xc.op == EQ) {
            string           auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";
            std::vector<int> tmp;
            tmp.push_back(xc.val);
            buildVariableInteger(auxVar, tmp);
            FactoryConstraints::createConstraintNValuesEQV(problem, id, toMyVariables(list), problem->mapping[auxVar]);
            return;
        }
        throw runtime_error("c Such nValues constraint Not yes supported");
    }
    if(xc.op == IN) {
        string auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";
        if(xc.operandType == INTERVAL)
            buildVariableInteger(auxVar, xc.min, xc.max);
        else
            buildVariableInteger(auxVar, xc.set);
        FactoryConstraints::createConstraintNValuesEQV(problem, id, toMyVariables(list), problem->mapping[auxVar]);
        return;
    }
    assert(xc.op == NOTIN);
    int              min  = INT_MAX;
    int              max  = INT_MIN;
    vec<Variable *> &vars = toMyVariables(list);
    for(Variable *x : vars) {
        if(x->minimum() < min)
            min = x->minimum();
        if(x->maximum() > max)
            max = x->maximum();
    }
    string     auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";
    XVariable *aux    = new XVariable(auxVar, nullptr);
    buildVariableInteger(auxVar, 1, max - min + 2);
    FactoryConstraints::createConstraintNValuesEQV(problem, id, toMyVariables(list), problem->mapping[auxVar]);
    if(xc.operandType == SET)
        buildConstraintExtension(id, aux, xc.set, false, false);
    else {
        std::vector<int> tmp;
        for(int i = xc.min; i <= xc.max; i++) tmp.push_back(i);
        buildConstraintExtension(id, aux, tmp, false, false);
    }
}

void CosocoCallbacks::buildConstraintNValues(string id, vector<Tree *> &trees, XCondition &xc) {
    vector<string> auxiliaryVariables;
    insideGroup = false;
    createAuxiliaryVariablesAndExpressions(trees, auxiliaryVariables);
    vector<XVariable *> tmp;
    for(auto &auxiliaryVariable : auxiliaryVariables) {
        tmp.push_back(new XVariable(auxiliaryVariable, nullptr));
    }
    buildConstraintNValues(id, tmp, xc);
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

std::string joinVector(std::vector<int> &array) {
    std::ostringstream oss;
    for(size_t i = 0; i < array.size(); ++i) {
        oss << array[i];
        if(i != array.size() - 1) {
            oss << ",";
        }
    }
    return oss.str();
}

void CosocoCallbacks::buildConstraintCount(string id, vector<Tree *> &trees, vector<XVariable *> &values, XCondition &xc) {
    // TODO WARNING
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

void CosocoCallbacks::buildConstraintCount(string id, vector<XVariable *> &list, vector<int> &values, XCondition &xc) {
    vector<Tree *> trees;
    if(values.size() == 1) {
        for(auto &xv : list) {
            string s = "eq(" + xv->id + "," + std::to_string(values[0]) + ")";
            trees.push_back(new Tree(s));
        }
        buildConstraintSum(id, trees, xc);
        return;
    }
    std::string tmp = joinVector(values);
    for(auto &xv : list) {
        string s = "in(" + xv->id + ",set(" + tmp + "))";
        std::cout << s << std::endl;
        trees.push_back(new Tree(s));
    }
    buildConstraintSum(id, trees, xc);
    return;
}


void CosocoCallbacks::buildConstraintAmong(string id, vector<XVariable *> &list, vector<int> &values, int k) {
    FactoryConstraints::createConstraintAmong(problem, id, toMyVariables(list), vector2vec(values), k);
}


void CosocoCallbacks::buildConstraintCardinality(string id, vector<XVariable *> &list, vector<int> values, vector<int> &intOccurs,
                                                 bool closed) {
    vec<Variable *> occurs;
    for(int value : intOccurs) {
        string auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";
        buildVariableInteger(auxVar, value, value);
        auto *x = new XVariable(auxVar, nullptr);
        occurs.push(problem->mapping[x->id]);
    }
    FactoryConstraints::createConstraintCardinality(problem, id, toMyVariables(list), vector2vec(values), occurs);
}


void CosocoCallbacks::buildConstraintCardinality(string id, vector<XVariable *> &list, vector<int> values,
                                                 vector<XVariable *> &varOccurs, bool closed) {
    vec<Variable *> occurs;
    for(auto &xv : varOccurs) {
        occurs.push(problem->mapping[xv->id]);
    }
    FactoryConstraints::createConstraintCardinality(problem, id, toMyVariables(list), vector2vec(values), occurs);
}


void CosocoCallbacks::buildConstraintCardinality(string id, vector<XVariable *> &list, vector<int> values,
                                                 vector<XInterval> &intervalOccurs, bool closed) {
    vec<Variable *> occurs;
    for(XInterval &xi : intervalOccurs) {
        string auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";
        buildVariableInteger(auxVar, xi.min, xi.max);
        auto *x = new XVariable(auxVar, nullptr);
        occurs.push(problem->mapping[x->id]);
    }
    FactoryConstraints::createConstraintCardinality(problem, id, toMyVariables(list), vector2vec(values), occurs);
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
    for(unsigned int i = 0; i < list.size(); i++) tmp += (i != 0 ? "," : "") + list[i]->id;
    tmp = tmp + "))";
    return tmp;
}


void CosocoCallbacks::buildConstraintMaximum(string id, vector<Tree *> &list, XCondition &xc) {
    if(xc.operandType == VARIABLE) {
        if(xc.op == EQ) {
            vector<string> auxiliaryVariables;
            insideGroup = false;
            createAuxiliaryVariablesAndExpressions(list, auxiliaryVariables);
            vars.clear();
            for(auto &auxiliaryVariable : auxiliaryVariables) vars.push(problem->mapping[auxiliaryVariable]);
            FactoryConstraints::createConstraintMaximumVariableEQ(problem, id, vars, problem->mapping[xc.var]);
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
            vars.clear();
            for(auto &auxiliaryVariable : auxiliaryVariables) vars.push(problem->mapping[auxiliaryVariable]);
            FactoryConstraints::createConstraintMinimumVariableEQ(problem, id, vars, problem->mapping[xc.var]);
            return;
        }
    }
    throw runtime_error("minimum over set is not yet supported");
}

void CosocoCallbacks::buildConstraintMaximum(string id, vector<XVariable *> &list, XCondition &xc) {
    if(xc.operandType == VARIABLE) {
        if(xc.op == EQ) {
            toMyVariables(list, vars);
            FactoryConstraints::createConstraintMaximumVariableEQ(problem, id, vars, problem->mapping[xc.var]);
        } else
            buildConstraintIntension(id, new Tree(createExpression("max", xc.op, list, xc.var)));
        return;
    }

    if(xc.operandType == INTEGER) {
        if(xc.op == LE || xc.op == LT) {
            toMyVariables(list, vars);
            FactoryConstraints::createConstraintMaximumLE(problem, id, vars, xc.op == LE ? xc.val : xc.val - 1);
            return;
        }
        if(xc.op == GE || xc.op == GT) {
            toMyVariables(list, vars);
            FactoryConstraints::createConstraintMaximumGE(problem, id, vars, xc.op == GE ? xc.val : xc.val + 1);
            return;
        }
        buildConstraintIntension(id, new XCSP3Core::Tree(createExpression("max", xc.op, list, std::to_string(xc.val))));
        return;
    }
    // Interval
    toMyVariables(list, vars);
    FactoryConstraints::createConstraintMaximumLE(problem, id, vars, xc.min);
    FactoryConstraints::createConstraintMaximumGE(problem, id, vars, xc.max);
}


void CosocoCallbacks::buildConstraintMinimum(string id, vector<XVariable *> &list, XCondition &xc) {
    if(xc.operandType == VARIABLE) {
        if(xc.op == EQ) {
            toMyVariables(list, vars);
            FactoryConstraints::createConstraintMinimumVariableEQ(problem, id, vars, problem->mapping[xc.var]);
        } else
            buildConstraintIntension(id, new XCSP3Core::Tree(createExpression("min", xc.op, list, xc.var)));
        return;
    }

    if(xc.operandType == INTEGER) {
        if(xc.op == LE || xc.op == LT) {
            toMyVariables(list, vars);
            FactoryConstraints::createConstraintMinimumLE(problem, id, vars, xc.op == LE ? xc.val : xc.val - 1);
            return;
        }
        if(xc.op == GE || xc.op == GT) {
            toMyVariables(list, vars);
            FactoryConstraints::createConstraintMinimumGE(problem, id, vars, xc.op == GE ? xc.val : xc.val + 1);
            return;
        }
        buildConstraintIntension(id, new XCSP3Core::Tree(createExpression("min", xc.op, list, std::to_string(xc.val))));
        return;
    }
    /// Interval
    toMyVariables(list, vars);
    FactoryConstraints::createConstraintMinimumLE(problem, id, vars, xc.min);
    FactoryConstraints::createConstraintMinimumGE(problem, id, vars, xc.max);
}


void CosocoCallbacks::buildConstraintElement(string id, vector<XVariable *> &list, int startIndex, XVariable *index,
                                             RankType rank, int value) {
    Variable *idx = problem->mapping[index->id];
    FactoryConstraints::createConstraintElementConstant(problem, id, toMyVariables(list), idx, startIndex, value);
}


void CosocoCallbacks::buildConstraintElement(string id, vector<XVariable *> &list, int startIndex, XVariable *index,
                                             RankType rank, XVariable *value) {
    if(value->id == index->id) {
        for(XVariable *x : list)
            if(x == value)
                throw runtime_error("Element variable value in list not yet implemented");


        vec<vec<int>> tuples;
        toMyVariables(list);
        vars.push(problem->mapping[value->id]);
        for(int i = 0; i < vars.size(); i++) {
            if(vars[i]->containsValue(i)) {
                tuples.push();
                for(int j = 0; j < vars.size(); j++) tuples.last().push(STAR);
                tuples.last()[i]     = i;
                tuples.last().last() = i;
            }
        }
        FactoryConstraints::createConstraintExtension(problem, id, vars, tuples, true, true);
        return;
    }

    Variable *idx = problem->mapping[index->id];
    Variable *val = problem->mapping[value->id];
    assert(val != nullptr);
    FactoryConstraints::createConstraintElementVariable(problem, id, toMyVariables(list), idx, startIndex, val);
}


void CosocoCallbacks::buildConstraintElement(string id, vector<int> &list, int startIndex, XVariable *index, RankType rank,
                                             XVariable *value) {
    vec<Variable *> tmp;
    vec<vec<int>>   tuples;
    Variable       *idx = problem->mapping[index->id];
    Variable       *val = problem->mapping[value->id];
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
    FactoryConstraints::createConstraintExtension(problem, id, tmp, tuples, true, false);
}

void CosocoCallbacks::buildConstraintElement(string id, vector<int> &list, XVariable *index, int startIndex, XCondition &xc) {
    vector<XVariable *> aux;
    for(int value : list) {
        string auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";
        buildVariableInteger(auxVar, value, value);
        auto *x = new XVariable(auxVar, nullptr);
        aux.push_back(x);
    }
    buildConstraintElement(id, aux, index, startIndex, xc);
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
    if(xc.operandType == INTERVAL)
        throw runtime_error("Element with condition and interval not yet  implemented");


    Variable *v   = problem->mapping[list[0]->id];
    int       min = v->minimum();
    int       max = v->maximum();
    for(unsigned int i = 1; i < list.size(); i++) {
        v   = problem->mapping[list[i]->id];
        min = min > v->minimum() ? v->minimum() : min;
        max = max < v->maximum() ? v->maximum() : max;
    }

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
    tmp     = tmp + auxVar + "," + (xc.operandType == VARIABLE ? xc.var : std::to_string(xc.val)) + ")";
    auto *x = new XVariable(auxVar, nullptr);
    buildConstraintIntension(id, new Tree(tmp));
    buildConstraintElement(id, list, startIndex, index, ANY, x);
}

void CosocoCallbacks::buildConstraintElement(string id, vector<vector<int>> &matrix, int startRowIndex, XVariable *rowIndex,
                                             int startColIndex, XVariable *colIndex, XVariable *value) {
    if(startRowIndex != 0 || startColIndex != 0)
        throw runtime_error("Element int matrix with startRowIndex or StartColIndex !=0 not yet supported");


    vec<vec<int>>   tuples;
    vec<Variable *> vars;
    Variable       *row = problem->mapping[rowIndex->id];
    Variable       *col = problem->mapping[colIndex->id];
    Variable       *val = problem->mapping[value->id];
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
    FactoryConstraints::createConstraintExtension(problem, id, vars, tuples, true, false);
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

    vec<vec<Variable *>> m2;


    for(unsigned int i = 0; i < matrix.size(); i++) {
        m2.push();
        for(unsigned int j = 0; j < matrix[i].size(); j++) {
            m2.last().push(problem->mapping[matrix[i][j]->id]);
        }
    }
    FactoryConstraints::createConstraintElementMatrix(problem, id, m2, problem->mapping[rowIndex->id],
                                                      problem->mapping[colIndex->id], value);
}

void CosocoCallbacks::buildConstraintElement(string id, vector<vector<XVariable *>> &matrix, int startRowIndex,
                                             XVariable *rowIndex, int startColIndex, XVariable *colIndex, XVariable *value) {
    vec<vec<Variable *>> m2;
    for(unsigned int i = 0; i < matrix.size(); i++) {
        m2.push();
        for(unsigned int j = 0; j < matrix[i].size(); j++) {
            m2.last().push(problem->mapping[matrix[i][j]->id]);
        }
    }

    FactoryConstraints::createConstraintElementMatrix(problem, id, m2, problem->mapping[rowIndex->id],
                                                      problem->mapping[colIndex->id], problem->mapping[value->id]);
}

void CosocoCallbacks::buildConstraintChannel(string id, vector<XVariable *> &list, int startIndex) {
    if(startIndex != 0)
        throw runtime_error("Channel is not implemented with index != 0");
    toMyVariables(list);
    FactoryConstraints::createConstraintChannel(problem, id, vars, 0);
}

void CosocoCallbacks::buildConstraintChannel(string id, vector<XVariable *> &list1, int startIndex1, vector<XVariable *> &list2,
                                             int startIndex2) {
    vec<Variable *> X;
    vec<Variable *> Y;
    toMyVariables(list1, X);
    toMyVariables(list2, Y);
    FactoryConstraints::createContraintChannelXY(problem, id, X, Y, startIndex1, startIndex2);
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
bool Disj::same(string xx, string yy, int llx, int lly) { return (x == xx && y == yy && lx == llx && ly == lly); }
Disj::Disj(string xx, string yy, int llx, int lly) {
    x  = xx;
    y  = yy;
    lx = llx;
    ly = lly;
}


void CosocoCallbacks::buildConstraintNoOverlap(string id, vector<XVariable *> &origins, vector<int> &lengths, bool zeroIgnored) {
    if(!zeroIgnored)
        throw runtime_error("Nooverlap with zeroIgnored not yet supported");

    toMyVariables(origins);
    for(int i = 0; i < vars.size(); i++)
        for(int j = i + 1; j < vars.size(); j++) {
            bool exist = false;
            for(auto *d : allDisjunctives)
                if(d->same(vars[i]->_name, vars[j]->_name, lengths[i], lengths[j])) {
                    exist = true;
                    break;
                }
            if(exist)
                continue;

            string auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";
            buildVariableInteger(auxVar, 0, 3);
            Variable *aux = problem->mapping[auxVar];

            FactoryConstraints::createConstraintDisjunctive(problem, id, vars[i], vars[j], lengths[i], lengths[j], aux);
            allDisjunctives.push(new Disj(vars[0]->_name, vars[1]->_name, lengths[i], lengths[j]));
        }
}

void CosocoCallbacks::buildConstraintNoOverlap(string id, vector<XVariable *> &origins, vector<XVariable *> &lengths,
                                               bool zeroIgnored) {
    if(!zeroIgnored)
        throw runtime_error("K dim Nooverlap with zeroIgnored not yet supported");
    for(unsigned int i = 0; i < origins.size(); i++)
        for(unsigned int j = i + 1; j < origins.size(); j++) {
            Variable *xi = problem->mapping[origins[i]->id];
            Variable *xj = problem->mapping[origins[j]->id];
            Variable *wi = problem->mapping[lengths[i]->id];
            Variable *wj = problem->mapping[lengths[j]->id];
            FactoryConstraints::createConstraintDisjunctiveVars(problem, id, xi, xj, wi, wj);
        }
}

void CosocoCallbacks::buildConstraintNoOverlap(string id, vector<vector<XVariable *>> &origins,
                                               vector<vector<XVariable *>> &lengths, bool zeroIgnored) {
    if(!zeroIgnored)
        throw runtime_error("K dim Nooverlap with zeroIgnored not yet supported");

    if(origins[0].size() == 3) {
        struct {
            string doit(Variable *a, Variable *b, Variable *c) {
                return "le(add(" + a->_name + "," + b->_name + ")," + c->_name + ")";
            }
        } le;

        for(unsigned int i = 0; i < origins.size(); i++) {
            for(unsigned int j = i + 1; j < origins.size(); j++) {
                Variable *xi = problem->mapping[origins[i][0]->id];
                Variable *xj = problem->mapping[origins[j][0]->id];

                Variable *yi = problem->mapping[origins[i][1]->id];
                Variable *yj = problem->mapping[origins[j][1]->id];

                Variable *zi = problem->mapping[origins[i][2]->id];
                Variable *zj = problem->mapping[origins[j][2]->id];

                Variable *wi = problem->mapping[lengths[i][0]->id];
                Variable *wj = problem->mapping[lengths[j][0]->id];

                Variable *hi = problem->mapping[lengths[i][1]->id];
                Variable *hj = problem->mapping[lengths[j][1]->id];

                Variable *di = problem->mapping[lengths[i][2]->id];
                Variable *dj = problem->mapping[lengths[j][2]->id];

                string tmp = "or(" + le.doit(xi, wi, xj) + "," + le.doit(xj, wj, xi) + "," + le.doit(yi, hi, yj) + "," +
                             le.doit(yj, hj, yi) + "," + le.doit(zi, di, zj) + "," + le.doit(zj, dj, zi) + ")";
                buildConstraintIntension(id, new Tree(tmp));
            }
        }
        return;
    }


    for(unsigned int i = 0; i < origins.size(); i++) {
        for(unsigned int j = i + 1; j < origins.size(); j++) {
            string auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";
            buildVariableInteger(auxVar, 0, 3);
            Variable *aux = problem->mapping[auxVar];
            Variable *xi  = problem->mapping[origins[i][0]->id];
            Variable *xj  = problem->mapping[origins[j][0]->id];
            Variable *yi  = problem->mapping[origins[i][1]->id];
            Variable *yj  = problem->mapping[origins[j][1]->id];
            Variable *wi  = problem->mapping[lengths[i][0]->id];
            Variable *wj  = problem->mapping[lengths[j][0]->id];
            Variable *hi  = problem->mapping[lengths[i][1]->id];
            Variable *hj  = problem->mapping[lengths[j][1]->id];


            FactoryConstraints::createConstraintDisjunctive2DVar(problem, id, xi, xj, yi, yj, wi, wj, hi, hj, aux);
        }
    }
}


void CosocoCallbacks::buildConstraintNoOverlap(string id, vector<vector<XVariable *>> &origins, vector<vector<int>> &lengths,
                                               bool zeroIgnored) {
    if(!zeroIgnored)
        throw runtime_error("K dim Nooverlap with zeroIgnored not yet supported");
    assert(origins.size() == lengths.size() && origins[0].size() == 2);
    if(true) {
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
            X.push(problem->mapping[origins[i][0]->id]);
            Y.push(problem->mapping[origins[i][1]->id]);
        }
        FactoryConstraints::createConstraintNoOverlap(problem, id, X, w, Y, h);
    }
    for(unsigned int i = 0; i < origins.size(); i++) {
        for(unsigned int j = i + 1; j < origins.size(); j++) {
            Variable *x1     = problem->mapping[origins[i][0]->id];
            Variable *x2     = problem->mapping[origins[j][0]->id];
            Variable *y1     = problem->mapping[origins[i][1]->id];
            Variable *y2     = problem->mapping[origins[j][1]->id];
            string    auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";
            buildVariableInteger(auxVar, 0, 3);
            Variable *aux = problem->mapping[auxVar];
            FactoryConstraints::createConstraintDisjunctive2D(problem, id, x1, x2, y1, y2, lengths[i][0], lengths[j][0],
                                                              lengths[i][1], lengths[j][1], aux);
        }
    }
    // Add redundant constraint
    vector<XVariable *> ox, oy;
    vector<int>         lx, ly;
    for(unsigned int i = 0; i < origins.size(); i++) {
        ox.push_back(origins[i][0]);
        lx.push_back(lengths[i][0]);
        oy.push_back(origins[i][1]);
        ly.push_back(lengths[i][1]);
    }
    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;
    for(unsigned int i = 0; i < origins.size(); i++) {
        minX = std::min(minX, ox[i]->domain->minimum());
        minY = std::min(minY, oy[i]->domain->minimum());
        maxX = std::max(maxX, ox[i]->domain->maximum() + lx[i]);
        maxY = std::max(maxY, oy[i]->domain->maximum() + ly[i]);
    }
    // string id, vector<XVariable *> &origins, vector<int> &lengths,
    //                                                vector<int> &heights, XCondition &xc)

    XCondition xc;
    xc.op  = LE;
    xc.val = maxY - minY;
    // string id, vector<XVariable *> &origins, vector<int> &lengths,
    //                                             vector<int> &heights, XCondition &xc
    buildConstraintCumulative(id, ox, lx, ly, xc);
    xc.val = maxX - minX;
    buildConstraintCumulative(id, oy, ly, lx, xc);
    // FactoryConstraints::createConstraintCumulative(problem, id, ox, lx, ly, maxY - minY);
    // FactoryConstraints::createConstraintCumulative(problem, id, oy, ly, lx, maxX - minX);

    return;


    /*for(unsigned int i = 0; i < origins.size(); i++) {
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
    }*/
}


void CosocoCallbacks::buildConstraintCumulative(string id, vector<XVariable *> &origins, vector<int> &lengths,
                                                vector<int> &heights, XCondition &xc) {
    vars.clear();
    vec<int> h, l;
    vector2vec(lengths);
    vals.copyTo(l);
    vector2vec(heights);
    vals.copyTo(h);
    toMyVariables(origins, vars);
    if(xc.operandType == VARIABLE)
        FactoryConstraints::createConstraintCumulative(problem, id, vars, l, h, problem->mapping[xc.var]);
    else
        FactoryConstraints::createConstraintCumulative(problem, id, vars, l, h, xc.val);
}


void CosocoCallbacks::buildConstraintCumulative(string id, vector<XVariable *> &origins, vector<int> &lengths,
                                                vector<XVariable *> &varHeights, XCondition &xc) {
    vars.clear();
    vec<int>        l;
    vec<Variable *> myvarHeights;

    vector2vec(lengths);
    vals.copyTo(l);
    toMyVariables(varHeights, myvarHeights);
    toMyVariables(origins, vars);
    if(xc.operandType == VARIABLE)
        FactoryConstraints::createConstraintCumulativeHeightVariableLV(problem, id, vars, l, myvarHeights,
                                                                       problem->mapping[xc.var]);
    else
        FactoryConstraints::createConstraintCumulativeHeightVariable(problem, id, vars, l, myvarHeights, xc.val);
}


void CosocoCallbacks::buildConstraintCumulative(string id, vector<XVariable *> &origins, vector<XVariable *> &varlengths,
                                                vector<int> &h, XCondition &xc) {
    vars.clear();
    vec<int>        heights;
    vec<Variable *> myvarwidths;

    vector2vec(h);
    vals.copyTo(heights);
    toMyVariables(varlengths, myvarwidths);
    toMyVariables(origins, vars);
    if(xc.operandType == VARIABLE) {
        throw std::runtime_error("Cumulative with variable lenghts and condition variable is not yet implemented");
    } else
        FactoryConstraints::createConstraintCumulativeWidthVariables(problem, id, vars, myvarwidths, heights, xc.val);
}


void CosocoCallbacks::buildConstraintCumulative(string id, vector<XVariable *> &origins, vector<XVariable *> &varwidths,
                                                vector<XVariable *> &varheights, XCondition &xc) {
    vars.clear();
    vec<Variable *> myvarwidths, myvarheights;

    toMyVariables(varwidths, myvarwidths);
    toMyVariables(varheights, myvarheights);
    toMyVariables(origins, vars);
    if(xc.operandType == VARIABLE) {
        FactoryConstraints::createConstraintCumulativeHeightAndWidthAndConditionVariables(problem, id, vars, myvarwidths,
                                                                                          myvarheights, problem->mapping[xc.var]);
    } else
        FactoryConstraints::createConstraintCumulativeHeightAndWidthVariables(problem, id, vars, myvarwidths, myvarheights,
                                                                              xc.val);
}


void CosocoCallbacks::buildConstraintBinPacking(string id, vector<XVariable *> &list, vector<int> &sizes, XCondition &cond) {
    vars.clear();
    toMyVariables(list, vars);
    if(Variable::haveSameDomainType(vars) == false) {
        throw std::runtime_error("Bin packing with different domain types for items is not yet implemented");
    }
    vec<int> s;
    vector2vec(sizes);
    vals.copyTo(s);

    if(cond.operandType == VARIABLE)
        throw std::runtime_error("Bin packing with variable in condition is not yet supported");
    if(cond.op != LE && cond.op != LT)
        throw std::runtime_error("Bin packing with condition not LE/LT is not yet supported");
    vec<int> limits;

    limits.growTo(vars[0]->domain.maxSize(), cond.val - (cond.op == LT ? 1 : 0));
    FactoryConstraints::createConstraintBinPacking(problem, id, vars, s, limits);
}


void CosocoCallbacks::buildConstraintBinPacking(string id, vector<XVariable *> &list, vector<int> &sizes, vector<int> &capacities,
                                                bool load) {
    vec<int> s, c;
    vector2vec(sizes);
    vals.copyTo(s);
    vector2vec(capacities);
    vals.copyTo(c);
    vars.clear();
    toMyVariables(list, vars);
    if(Variable::haveSameDomainType(vars) == false) {
        throw std::runtime_error("Bin packing with different domain types for items is not yet implemented");
    }
    if(load) {
        vec<Variable *> loads;
        for(int cap : capacities) {
            string auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";
            buildVariableInteger(auxVar, cap, cap);
            loads.push(problem->variables.last());
        }
        FactoryConstraints::createConstraintBinPacking(problem, id, vars, s, loads);
    } else
        FactoryConstraints::createConstraintBinPacking(problem, id, vars, s, c);
}

void CosocoCallbacks::buildConstraintBinPacking(string id, vector<XVariable *> &list, vector<int> &sizes,
                                                vector<XVariable *> &capacities, bool load) {
    vec<int> s;
    vector2vec(sizes);
    vals.copyTo(s);
    vars.clear();
    toMyVariables(list, vars);
    vec<Variable *> loads;
    toMyVariables(capacities, loads);
    if(load) {
        FactoryConstraints::createConstraintBinPacking(problem, id, vars, s, loads);
    } else {
        set<int> b;
        for(Variable *x : vars)
            for(int idv : x->domain) b.insert(x->domain.toVal(idv));
        vector<int> bins;
        bins.assign(b.begin(), b.end());
        for(int bin : bins) {
            vector<Tree *> trees;
            for(XVariable *x : list) trees.push_back(new Tree("eq(" + x->id + "," + std::to_string(bin) + ")"));
            XCondition xc;
            xc.op          = LE;
            xc.operandType = VARIABLE;
            xc.var         = capacities[bin]->id;
            buildConstraintSum(id, trees, sizes, xc);
        }
    }
}

void CosocoCallbacks::buildConstraintBinPacking(string id, vector<XVariable *> &list, vector<int> &sizes,
                                                vector<XCondition> &conditions, int startindex) {
    for(XCondition &xc : conditions) {
        if(xc.operandType != VARIABLE && xc.op != EQ)
            throw std::runtime_error("Bin packing with all condiions not EQ=VAR is not yet implemented");
    }
    vars.clear();
    vec<int> s;
    vector2vec(sizes);
    vals.copyTo(s);
    toMyVariables(list, vars);
    vec<Variable *> loads;
    for(XCondition &xc : conditions) loads.push(problem->mapping[xc.var]);
    FactoryConstraints::createConstraintBinPacking(problem, id, vars, s, loads);
}

//--------------------------------------------------------------------------------------
// Instantiation constraint
//--------------------------------------------------------------------------------------

void CosocoCallbacks::buildConstraintInstantiation(string id, vector<XVariable *> &list, vector<int> &values) {
    for(unsigned int i = 0; i < list.size(); i++) {
        vec<int> value;
        if(values[i] == STAR)
            continue;
        value.push(values[i]);
        FactoryConstraints::createConstraintUnary(problem, id, problem->mapping[list[i]->id], value, true);
    }
}


void CosocoCallbacks::buildConstraintPrecedence(string id, vector<XVariable *> &list, vector<int> values, bool covered) {
    toMyVariables(list);
    FactoryConstraints::createConstraintPrecedence(problem, id, vars, vector2vec(values), covered);
}

void CosocoCallbacks::buildConstraintPrecedence(string id, vector<XVariable *> &list, bool covered) {
    toMyVariables(list);
    std::set<int> values;
    for(Variable *x : vars) {
        for(int idv : x->domain) values.insert(x->domain.toVal(idv));
    }
    vec<int> tmp;
    for(int v : values) tmp.push(v);
    FactoryConstraints::createConstraintPrecedence(problem, id, vars, tmp, covered);
}

void CosocoCallbacks::buildConstraintKnapsack(string id, vector<XVariable *> &list, vector<int> &weights, vector<int> &profits,
                                              XCondition weightsCondition, XCondition &profitCondition) {
    buildConstraintSum(id, list, weights, weightsCondition);
    buildConstraintSum(id, list, profits, profitCondition);
}


void CosocoCallbacks::buildConstraintFlow(string id, vector<XVariable *> &list, vector<int> &balance, vector<int> &weights,
                                          vector<vector<int>> &arcs, XCondition &xc) {
    std::set<int> set;
    toMyVariables(list);
    for(auto &arc : arcs) {
        set.insert(arc[0]);
        set.insert(arc[1]);
    }
    vec<int> nodes;
    for(int v : set) nodes.push(v);

    int sm = nodes[0];

    vec<vec<Variable *>> preds(nodes.size());
    vec<vec<Variable *>> succs(nodes.size());

    for(unsigned int i = 0; i < arcs.size(); i++) {
        preds[arcs[i][1] - sm].push(vars[i]);
        succs[arcs[i][0] - sm].push(vars[i]);
    }
    vec<Variable *> tmp;
    for(int i = 0; i < nodes.size(); i++) {
        tmp.clear();
        succs[i].copyTo(tmp);
        tmp.extend(preds[i]);
        vec<int> coeffs;
        coeffs.growTo(succs[i].size(), 1);
        for(int j = 0; j < preds[i].size(); j++) coeffs.push(-1);
        FactoryConstraints::createConstraintSum(problem, id, tmp, coeffs, balance[i], EQ);
    }
    buildConstraintSum(id, list, weights, xc);
}


//--------------------------------------------------------------------------------------
// Objectives
//--------------------------------------------------------------------------------------

void CosocoCallbacks::buildObjectiveMinimizeExpression(string expr) {
    string tmp = "le(" + expr + ",0)";
    buildConstraintIntension("objective", new XCSP3Core::Tree(tmp));

    auto *po = static_cast<OptimizationProblem *>(problem);
    po->type = OptimisationType::Minimize;
    auto *oc = dynamic_cast<ObjectiveConstraint *>(problem->constraints.last());
    po->addObjectiveUB(oc, true);
}


void CosocoCallbacks::buildObjectiveMaximizeExpression(string expr) {
    string tmp = "ge(" + expr + ",0)";   // Fake value
    buildConstraintIntension("objective", new XCSP3Core::Tree(tmp));
    auto *po = static_cast<OptimizationProblem *>(problem);
    po->type = OptimisationType::Maximize;
    auto *oc = dynamic_cast<ObjectiveConstraint *>(problem->constraints.last());
    assert(oc != nullptr);
    po->addObjectiveLB(oc, true);
}

void CosocoCallbacks::buildObjectiveMinimizeVariable(XVariable *x) {
    FactoryConstraints::createConstraintUnaryLE(problem, "", problem->mapping[x->id], 0);
    auto *po = static_cast<OptimizationProblem *>(problem);
    po->type = OptimisationType::Minimize;
    auto *oc = dynamic_cast<ObjectiveConstraint *>(problem->constraints.last());
    po->addObjectiveUB(oc, true);
}

void CosocoCallbacks::buildObjectiveMaximizeVariable(XVariable *x) {
    FactoryConstraints::createConstraintUnaryGE(problem, "", problem->mapping[x->id], 0);
    auto *po = static_cast<OptimizationProblem *>(problem);
    po->type = OptimisationType::Maximize;
    auto *oc = dynamic_cast<ObjectiveConstraint *>(problem->constraints.last());
    po->addObjectiveLB(oc, true);
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


void CosocoCallbacks::buildObjectiveMinimize(ExpressionObjective type, vec<Variable *> &variables, vector<int> &origcoeffs) {
    auto *po = static_cast<OptimizationProblem *>(problem);
    po->type = OptimisationType::Minimize;

    switch(type) {
        case EXPRESSION_O:
            break;   // useless, want to discard warning
        case SUM_O: {
            // Change optim type because LE->GE in Sum
            po->type           = OptimisationType::Maximize;
            invertOptimization = true;

            FactoryConstraints::createConstraintSum(problem, "objective", variables, vector2vec(origcoeffs), INT_MAX, LE);
            auto *oc = dynamic_cast<ObjectiveConstraint *>(problem->constraints.last());
            po->addObjectiveLB(oc, true);
            break;
        }
        case PRODUCT_O:
            throw runtime_error("Objective product not yet supported");
            break;
        case MINIMUM_O: {
            FactoryConstraints::createConstraintMinimumLE(problem, "objective", variables, INT_MAX);
            auto *oc = dynamic_cast<ObjectiveConstraint *>(problem->constraints.last());
            po->addObjectiveUB(oc, true);
            break;
        }
        case MAXIMUM_O: {
            FactoryConstraints::createConstraintMaximumLE(problem, "objective", variables, INT_MAX);
            auto *oc = dynamic_cast<ObjectiveConstraint *>(problem->constraints.last());
            po->addObjectiveUB(oc, true);
            break;
        }
        case NVALUES_O: {
            FactoryConstraints::createConstraintNValuesLE(problem, "objective", variables, variables.size() + 1);
            auto *oc = dynamic_cast<ObjectiveConstraint *>(problem->constraints.last());
            po->addObjectiveUB(oc, true);
            break;
        }
        case LEX_O:
            throw runtime_error("Objective expression not yet supported");
            break;
    }
}


void CosocoCallbacks::buildObjectiveMinimize(ExpressionObjective type, vector<XVariable *> &list, vector<int> &origcoeffs) {
    toMyVariables(list, vars);
    buildObjectiveMinimize(type, vars, origcoeffs);
}


void CosocoCallbacks::buildObjectiveMaximize(ExpressionObjective type, vec<Variable *> &variables, vector<int> &origcoeffs) {
    auto *po = static_cast<OptimizationProblem *>(problem);
    po->type = OptimisationType::Maximize;

    switch(type) {
        case EXPRESSION_O:
            break;   // useless, want to discard warning
        case SUM_O: {
            FactoryConstraints::createConstraintSum(problem, "objective", variables, vector2vec(origcoeffs), INT_MIN, GE);
            break;
        }
        case PRODUCT_O:
            throw runtime_error("Objective product not yet supported");
            break;
        case MINIMUM_O:
            FactoryConstraints::createConstraintMinimumGE(problem, "objective", variables, INT_MIN);
            break;
        case MAXIMUM_O:
            FactoryConstraints::createConstraintMaximumGE(problem, "objective", variables, INT_MIN);
            break;
        case NVALUES_O:
            throw runtime_error("Objective expression not yet supported");
            break;
        case LEX_O:
            throw runtime_error("Objective expression not yet supported");
            break;
    }
    auto *oc = dynamic_cast<ObjectiveConstraint *>(problem->constraints.last());
    po->addObjectiveLB(oc, true);
}


void CosocoCallbacks::buildObjectiveMaximize(ExpressionObjective type, vector<XVariable *> &list, vector<int> &origcoeffs) {
    toMyVariables(list, vars);
    buildObjectiveMaximize(type, vars, origcoeffs);
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

        if(problem->mapping.find(predicate) == problem->mapping.end()) {   // This is a real expression
            string auxVar = "__av" + std::to_string(auxiliaryIdx++) + "__";

            if(problem->mapping.find(auxVar) != problem->mapping.end())
                throw runtime_error("Problem during creation of aux vars");
            auxiliaryVariables.push_back(auxVar);
            expressionsToAuxiliaryVariables[predicate] = auxVar;

            if(tree->listOfVariables.size() == 1 && XCSP3Core::isPredicateOperator(tree->root->type) == false) {
                // Add this part.
                Variable             *x = problem->mapping[tree->listOfVariables[0]];
                std::set<int>         values;
                std::map<string, int> tuple;
                for(int idv : x->domain) {
                    tuple[x->_name] = x->domain.toVal(idv);
                    values.insert(tree->evaluate(tuple));
                }
                std::vector<int> v(values.begin(), values.end());
                buildVariableInteger(auxVar, v);
            } else {
                Range r = possibleValuesForExpressionInRange(tree->root);
                buildVariableInteger(auxVar, r.min, r.max);
            }
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
    startToParseObjective = false;
    createAuxiliaryVariablesAndExpressions(trees, auxiliaryVariables);
    startToParseObjective = true;
    // Create the new objective
    // core duplication is here
    vars.clear();
    for(auto &auxiliaryVariable : auxiliaryVariables) vars.push(problem->mapping[auxiliaryVariable]);
    buildObjectiveMinimize(type, vars, coefs);
}


void CosocoCallbacks::buildObjectiveMaximize(ExpressionObjective type, vector<Tree *> &trees, vector<int> &coefs) {
    vector<string> auxiliaryVariables;

    startToParseObjective = false;
    createAuxiliaryVariablesAndExpressions(trees, auxiliaryVariables);
    startToParseObjective = false;

    // core duplication is here
    vars.clear();
    for(auto &auxiliaryVariable : auxiliaryVariables) vars.push(problem->mapping[auxiliaryVariable]);
    buildObjectiveMaximize(type, vars, coefs);
}


void CosocoCallbacks::buildObjectiveMinimize(ExpressionObjective type, vector<XVariable *> &list, vector<XVariable *> &coefs) {
    auto *po = static_cast<OptimizationProblem *>(problem);
    po->type = OptimisationType::Minimize;
    switch(type) {
        case SUM_O: {
            vector<Tree *> trees;
            for(unsigned int i = 0; i < list.size(); i++)
                trees.push_back(new Tree("mul(" + list[i]->id + "," + coefs[i]->id + ")"));
            vector<int> c;
            c.assign(trees.size(), -1);
            po->type           = OptimisationType::Maximize;
            invertOptimization = true;
            XCondition xc;
            xc.op          = GE;
            xc.operandType = INTEGER;
            xc.val         = INT_MIN;
            buildConstraintSum("objective", trees, c, xc);
            auto *oc = dynamic_cast<ObjectiveConstraint *>(problem->constraints.last());
            std::cout << problem->constraints.last()->type << std::endl;
            po->addObjectiveLB(oc, true);
            break;
        }
        default:
            runtime_error("Objective expression without sum and variables coeffs not yet supported");
    }
}


void CosocoCallbacks::buildObjectiveMaximize(ExpressionObjective type, vector<XVariable *> &list, vector<XVariable *> &coefs) {
    auto *po = static_cast<OptimizationProblem *>(problem);
    po->type = OptimisationType::Maximize;
    toMyVariables(list);
    vec<Variable *> c;
    toMyVariables(coefs, c);
    switch(type) {
        case SUM_O: {
            XCondition xc;
            xc.op          = GE;
            xc.operandType = INTEGER;
            xc.val         = INT_MIN;
            buildConstraintSum("objective", list, coefs, xc);
            break;
        }
        default:
            runtime_error("Objective expression without sum and variables coeffs not yet supported");
    }
    auto *oc = dynamic_cast<ObjectiveConstraint *>(problem->constraints.last());
    po->addObjectiveLB(oc, true);
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


void CosocoCallbacks::buildAnnotationDecision(vector<XVariable *> &list) { toMyVariables(list, decisionVariables); }
