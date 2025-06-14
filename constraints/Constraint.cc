#include "Constraint.h"

#include <assert.h>

#include <utility>

#include "XCSP3Tree.h"
#include "mtl/Map.h"
#include "solver/utils/FactoryConstraints.h"
#include "solver/utils/Options.h"
#include "utils/Utils.h"


using namespace std;
using namespace Cosoco;


// Constructors and delayed initialisation
Constraint::Constraint(Problem &p, std::string n)
    : problem(p), solver(nullptr), name(std::move(n)), unassignedVariablesIdx(), timestamp(0), wdeg(0) { }

Constraint::Constraint(Problem &p, std::string n, vec<Variable *> &vars)
    : problem(p), solver(nullptr), name(std::move(n)), unassignedVariablesIdx(), timestamp(0), wdeg(0) {
    addToScope(vars);
}


bool Constraint::isCorrectlyDefined() { return true; }


bool Constraint::scopeIsOk() {
    for(Variable *x : scope) x->fake = 0;
    for(Variable *x : scope) {
        if(x->fake == 1)
            throw std::logic_error("Constraint " + std::to_string(idc) + " " + type + " : scope contains variable " + x->name() +
                                   " many times");
        x->fake = 1;
    }
    return true;
}

void Constraint::addToScope(Variable *x) { scope.push(x); }


void Constraint::addToScope(vec<Variable *> &vars) {
    for(Variable *v : vars) scope.push(v);
}


void Constraint::delayedConstruction(int id) {
    assert(scope.size() > 0);   // scopeInitialisation have to be done
    makeDelayedConstruction(id);
}

#define MAXVARIABLESFORIDX 2000

void Constraint::makeDelayedConstruction(int id) {
    assert(scope.size() > 0);   // scopeInitialisation have to be done
    arity = scope.size();

    if(arity > 1 && problem.variables.size() < MAXVARIABLESFORIDX)
        idxToScopePositionArray.growTo(problem.variables.size(), NOTINSCOPE);


    unassignedVariablesIdx.setCapacity(arity, true);
    assert(unassignedVariablesIdx.size() == arity);
    if(arity > 1)
        for(int i = 0; i < scope.size(); i++) {
            if(problem.variables.size() < MAXVARIABLESFORIDX)
                idxToScopePositionArray[scope[i]->idx] = i;
            else
                idxToScopePositionMap.insert({scope[i]->idx, i});
        }

    current.growTo(scope.size());
    wdeg.growTo(scope.size());
    idc = id;
    for(Variable *v : scope) v->addConstraint(this);
}


void Constraint::attachSolver(Solver *s) { solver = s; }


// Filtering

State Constraint::status() { return UNDEF; }   // By default, a constraint must be filtering at each call

void Constraint::reinitialize() { }   // By default, nothing to do

bool Constraint::filterFrom(Variable *x) {
    State st2 = status();
    if(st2 == CONSISTENT)
        return true;
    if(st2 == INCONSISTENT)
        return false;
    return filter(x);
}

bool Constraint::postpone() {
    return isPostponable && options::intOptions["postponesize"].value > 0 &&
           scope.size() > options::intOptions["postponesize"].value;
}

// Assign and unassign variables
void Constraint::assignVariable(Variable *x) {
    int posx = toScopePosition(x->idx);
    assert(posx >= 0 && posx < scope.size());
    assert(unassignedVariablesIdx.contains(posx));
    unassignedVariablesIdx.del(posx);
}


void Constraint::unassignVariable(Variable *x) {
    int posInScope = toScopePosition(x->idx);
    assert(posInScope < scope.size());
    assert(!unassignedVariablesIdx.contains(posInScope));
    unassignedVariablesIdx.add(posInScope);
}


// idx -> scope position
int Constraint::toScopePosition(int idx) {
    if(arity == 1)
        return scope[0]->idx == idx ? 0 : NOTINSCOPE;

    if(problem.variables.size() < MAXVARIABLESFORIDX)
        return idxToScopePositionArray[idx];
    auto it = idxToScopePositionMap.find(idx);
    if(it == idxToScopePositionMap.end())
        return NOTINSCOPE;
    return it->second;
}


bool Constraint::isSatisfiedByOfIndexes(vec<int> &tupleOfIndex) {
    // if(indexesAreValues)
    //    return isSatisfiedBy(tupleOfIndex);
    // std::cout << tupleOfIndex.size() << " " << scope[2]->_name << "\n";
    for(int i = 0; i < scope.size(); i++) current[i] = scope[i]->domain.toVal(tupleOfIndex[i]);

    return isSatisfiedBy(current);
}


void Constraint::extractConstraintTupleFromInterpretation(const vec<int> &interpretation, vec<int> &tuple) {
    tuple.clear();
    for(auto &x : scope) {
        int idx = x->idx;
        tuple.push(interpretation[idx]);
    }
}


void createTuples(int posx, vec<Variable *> &scope, XCSP3Core::Tree *tree, vec<vec<int>> &conflicts, vec<vec<int>> &supports,
                  std::map<string, int> &tuple) {
    Variable *x = scope[posx];


    for(int i = 0; i < x->domain.maxSize(); i++) {
        tuple[x->_name] = x->domain.toVal(i);
        if(posx == scope.size() - 2 && tree->root->type == OEQ && tree->root->parameters[1]->type == OVAR) {
            auto *nv = dynamic_cast<NodeVariable *>(tree->root->parameters[1]);
            if(nodeContainsVar(tree->root->parameters[0], nv->var) == false) {
                // we have expr = var
                // we evaluate the expression and put the value in support
                int eval = tree->root->parameters[0]->evaluate(tuple);
                if(scope.last()->containsValue(eval)) {
                    supports.push();
                    tuple[scope.last()->_name] = eval;
                    assert(tuple.size() == scope.size());
                    for(auto &x : scope) supports.last().push(tuple[x->_name]);
                }
                continue;
            }
        }
        if(posx == scope.size() - 1) {
            vec<vec<int>> &putInside = tree->evaluate(tuple) ? supports : conflicts;
            putInside.push();
            for(auto &x : scope) {
                putInside.last().push(tuple[x->_name]);
            }
        } else
            createTuples(posx + 1, scope, tree, conflicts, supports, tuple);
    }
}


void Constraint::toExtensionConstraint(XCSP3Core::Tree *tree, vec<Variable *> &scope, std::vector<std::vector<int>> &tuples,
                                       bool &isSupport) {   //
    vec<vec<int>>         conflicts, supports;
    std::map<string, int> tuple;
    createTuples(0, scope, tree, conflicts, supports, tuple);

    // std::cout << conflicts.size() << "  " << supports.size() << std::endl;
    vec<vec<int>> &toCopy =
        (supports.size() > 0 && conflicts.size() == 0) || (conflicts.size() > supports.size()) ? supports : conflicts;
    isSupport = (supports.size() > 0 && conflicts.size() == 0) || (conflicts.size() > supports.size());
    tuples.clear();

    for(int i = 0; i < toCopy.size(); i++) {
        tuples.push_back(std::vector<int>());
        for(int j = 0; j < toCopy[i].size(); j++) tuples.back().push_back(toCopy[i][j]);
    }
}


// Display
void Constraint::display(bool allDetails) {
    printf("Constraint {%s} %s : [", (typeid(*this).name()) + 9, name.c_str());   // 9 to remove namespace...
    for(int i = 0; i < scope.size(); i++) printf(" %s ", scope[i]->name());
    printf("]\n");
}

//----------------- Simplify Constraint scope initialisation

vecVariables Constraint::temporary;
