#include "core/Problem.h"

#include "constraints/extensions/Extension.h"
#include "constraints/genericFiltering/AC3rm.h"

using namespace std;
using namespace Cosoco;


Problem::Problem(std::string n) : name(n), isConstructionDone(false) { }


void Problem::delayedConstruction() {
    if(isConstructionDone)
        throw std::logic_error("Construction of the problem is already done! You can not do twice\n");

    isConstructionDone = true;

    int idx = 0;
    for(Variable *v : variables) v->delayedConstruction(idx++, variables.size());

    int idc = 0;
    for(Constraint *c : constraints) c->delayedConstruction(idc++);

    bool error = false;
    try {
        for(Constraint *c : constraints) {
            c->scopeIsOk();
            c->isCorrectlyDefined();
        }
    } catch(const std::exception &e) {
        std::cerr << "\t" << e.what() << std::endl;
        error = true;
    }
    if(error)
        throw std::logic_error("Some constraints are badly defined");
    isBinary = maximumArity() == 2;

    for(Variable *x : variables) {
        if(x->constraints.size() == 0)
            x->useless = true;
    }
}


void Problem::attachSolver(Solver *s) {
    solver = s;
    for(Constraint *c : constraints) c->attachSolver(solver);
}


void Problem::addConstraint(Constraint *c) {
    if(isConstructionDone)
        throw std::logic_error("Construction of the problem is already done! You can not add constraints");
    constraints.push(c);
}


Variable *Problem::createVariable(std::string n, Domain &d, int array) {
    if(isConstructionDone)
        throw std::logic_error("Construction of the problem is already done! You can not add variables");
    auto *v = new Variable(*this, n, d, nbVariables(), array);
    variables.push(v);
    mapping[v->name()] = v;
    return v;
}


// Check Solution
bool Problem::checkSolution() {
    vec<int> tuple;
    // All variables must be assigned
    for(Variable *v : variables)
        if(v->size() != 1 && v->useless == false) {
            fprintf(stderr, "Solution Error : var %s has domain of size %d\n", v->name(), v->size());
            return false;
        }

    for(Constraint *c : constraints) {
        tuple.clear();
        for(Variable *x : c->scope) tuple.push(x->domain.toVal(x->domain[0]));

        if(c->isSatisfiedBy(tuple) == false) {
            for(Variable *x : c->scope)
                std::cout << x->name() << " " << x->value() << " " << x->domain.toVal(x->domain[0]) << endl;
            c->display();
            fprintf(stderr, "Solution Error : constraint number %d (name %s) is not valid\n", c->idc, c->name.c_str());
            exit(1);
        }
    }

    return true;
}

// Problem Statistics

int Problem::nbVariables() const { return variables.size(); }


int Problem::nbConstraints() const { return constraints.size(); }


int Problem::maximumTuplesInExtension() {
    int tmp = 0;
    for(Constraint *c : constraints) {
        Extension *ext;
        if((ext = dynamic_cast<Extension *>(c)) == nullptr)
            continue;
        if(tmp < ext->nbTuples())
            tmp = ext->nbTuples();
    }
    return tmp;
}


int Problem::minimumTuplesInExtension() {
    int tmp = INT_MAX;
    for(Constraint *c : constraints) {
        Extension *ext;
        if((ext = dynamic_cast<Extension *>(c)) == nullptr)
            continue;
        if(tmp > ext->nbTuples())
            tmp = ext->nbTuples();
    }
    return tmp;
}


int Problem::nbConstraintsOfSize(int size) {
    int nb = 0;
    for(Constraint *c : constraints)
        if(c->scope.size() == size)
            nb++;
    return nb;
}


int Problem::minimumArity() {
    int tmp = constraints[0]->scope.size();
    for(Constraint *c : constraints)
        if(c->scope.size() < tmp)
            tmp = c->scope.size();
    return tmp;
}


int Problem::maximumArity() {
    int tmp = 0;
    for(Constraint *c : constraints)
        if(c->scope.size() > tmp)
            tmp = c->scope.size();
    return tmp;
}


int Problem::maximumDomainSize() {
    int nb = variables[0]->size();
    for(Variable *v : variables)
        if(nb < v->size())
            nb = v->size();
    return nb;
}


int Problem::minimumDomainSize() {
    int nb = variables[0]->size();
    for(Variable *v : variables)
        if(nb > v->size())
            nb = v->size();
    return nb;
}


void Problem::nbTypeOfConstraints(std::map<std::string, int> &tmp) {
    for(Constraint *c : constraints) tmp[c->type]++;
}


// displayCurrentBranch
void Problem::display(bool alldetails) {
    printf("Variables : \n");
    for(Variable *v : variables) {
        printf(" ");
        v->display(true);
        printf("\n");
    }
    printf("Constraint : \n");
    for(Constraint *c : constraints) {
        printf(" ");
        c->display(true);
        printf("\n");
    }
}
