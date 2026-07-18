#include "core/Variable.h"

#include "constraints/Constraint.h"

using namespace std;
using namespace Cosoco;


// Construction and Initialisation
Variable::Variable(Problem &p, std::string n, Domain &d, int id, int a)
    : timestamp(0), problem(p), idx(id), array(a), _name(n), domain(d), wdeg(1), useless(false) { }


void Variable::delayedConstruction(int id, int nbVars) {
    domain.delayedConstruction(nbVars);
    if(idx != id)
        throw std::runtime_error("Problem construction : variable id is not the good one");
}


void Variable::addConstraint(Constraint *c) { constraints.push(c); }


// Delete Methods
void Variable::delIdv(int idv, int level) {   // Do not use directly, use solver's one
    domain.delIdv(idv, level);
}

void Variable::delValuesGE(int v, int lvl) { }

void Variable::delValuesLE(int v, int lvl) { }


// Assign methods
void Variable::assignToIdv(int idv, int level) { domain.reduceTo(idv, level); }


void Variable::assignToVal(int v, int level) { domain.reduceTo(domain.toIdv(v), level); }


// Display
void Variable::display(bool allDetails) {
    printf("Variable %s\n", name());
    if(allDetails)
        for(Constraint *c : constraints) {
            printf("     ");
            c->display(true);
        }
    printf("     ");
    domain.display();
}


// utils
bool Variable::haveSameDomainType(vec<Variable *> &vars) {
    size_t h = vars[0]->domain.hash();
    for(Variable *v : vars)
        if(h != v->domain.hash())
            return false;
    return true;
}

int Variable::maxDomainSize(vec<Variable *> &list) {
    int tmp = 0;
    for(Variable *x : list)
        if(x->size() > tmp)
            tmp = x->size();
    return tmp;
}


int Variable::sumDomainSize(vec<Variable *> &list) {
    int tmp = 0;
    for(Variable *x : list) tmp += x->size();
    return tmp;
}