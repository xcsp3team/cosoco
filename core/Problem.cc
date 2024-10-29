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
    for(auto &v : variables) v->delayedConstruction(idx++, variables.size());

    int idc = 0;
    for(auto &c : constraints) c->delayedConstruction(idc++);

    bool error = false;
    try {
        for(auto &c : constraints) {
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

    for(auto &x : variables) {
        if(x->constraints.size() == 0)
            x->useless = true;
    }
}


void Problem::attachSolver(Solver *s) {
    solver = s;
    for(auto &c : constraints) c->attachSolver(solver);
}


void Problem::addConstraint(std::unique_ptr<Constraint> &&c) {
    if(isConstructionDone)
        throw std::logic_error("Construction of the problem is already done! You can not add constraints");
    constraints.emplace_back(std::move(c));
}


Variable *Problem::createVariable(std::string n, Domain &d, int array) {
    if(isConstructionDone)
        throw std::logic_error("Construction of the problem is already done! You can not add variables");

    variables.emplace_back(std::make_unique<Variable>(*this, n, d, nbVariables(), array));
    auto *v            = variables.back().get();
    mapping[v->name()] = v;
    return v;
}


// Check Solution
bool Problem::checkSolution() {
    vec<int> tuple;
    // All variables must be assigned
    for(auto &v : variables)
        if(v->size() != 1 && v->useless == false) {
            fprintf(stderr, "Solution Error : var %s has domain of size %d\n", v->name(), v->size());
            return false;
        }

    for(auto &c : constraints) {
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
    unsigned int tmp = 0;
    for(auto &c : constraints) {
        Extension *ext;
        if((ext = dynamic_cast<Extension *>(c.get())) == nullptr)
            continue;
        if(tmp < ext->nbTuples())
            tmp = ext->nbTuples();
    }
    return tmp;
}


int Problem::minimumTuplesInExtension() {
    unsigned int tmp = INT_MAX;
    for(auto &c : constraints) {
        Extension *ext;
        if((ext = dynamic_cast<Extension *>(c.get())) == nullptr)
            continue;
        if(tmp > ext->nbTuples())
            tmp = ext->nbTuples();
    }
    return tmp;
}


int Problem::nbConstraintsOfSize(int size) {
    int nb = 0;
    for(auto &c : constraints)
        if(c->scope.size() == size)
            nb++;
    return nb;
}

long Problem::nbValues() {
    long tmp = 0;
    for(auto &x : variables) tmp += x->size();
    return tmp;
}

int Problem::minimumArity() {
    int tmp = constraints[0]->scope.size();
    for(auto &c : constraints)
        if(c->scope.size() < tmp)
            tmp = c->scope.size();
    return tmp;
}


int Problem::maximumArity() {
    int tmp = 0;
    for(auto &c : constraints)
        if(c->scope.size() > tmp)
            tmp = c->scope.size();
    return tmp;
}


int Problem::maximumDomainSize() {
    int nb = variables[0]->size();
    for(auto &v : variables)
        if(nb < v->size())
            nb = v->size();
    return nb;
}


int Problem::minimumDomainSize() {
    int nb = variables[0]->size();
    for(auto &v : variables)
        if(nb > v->size())
            nb = v->size();
    return nb;
}


void Problem::nbTypeOfConstraints(std::map<std::string, int> &tmp) {
    for(auto &c : constraints) tmp[c->type]++;
}


// displayCurrentBranch
void Problem::display(bool alldetails) {
    printf("Variables : \n");
    for(auto &v : variables) {
        printf(" ");
        v->display(true);
        printf("\n");
    }
    printf("Constraint : \n");
    for(auto &c : constraints) {
        printf(" ");
        c->display(true);
        printf("\n");
    }
}
