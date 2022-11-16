#include "core/Variable.h"

#include "constraints/Constraint.h"

using namespace std;
using namespace Cosoco;


// Construction and Initialisation
Variable::Variable(Problem &p, std::string n, Domain &d, int id, int a)
    : addToTrail(false), timestamp(0), problem(p), idx(id), array(a), _name(n), domain(d), wdeg(1), useless(false) { }


void Variable::delayedConstruction(int id, int nbVars) {
    domain.delayedConstruction(nbVars);
    if(idx != id)
        throw std::runtime_error("Problem construction : variable id is not the good one");
}


void Variable::addConstraint(Constraint *c) { constraints.push(c); }


// Delete Methods
bool Variable::delVal(int v, int level) {   // Do not use directly, use solver's one
    if(domain.idvs.isLimitRecordedAtLevel(level) == false) {
        addToTrail = true;
        domain.idvs.recordLimit(level);
    }
    domain.delVal(v, level);
    return size() == 0;
}


bool Variable::delIdv(int idv, int level) {   // Do not use directly, use solver's one
    if(domain.idvs.isLimitRecordedAtLevel(level) == false) {
        addToTrail = true;
        domain.idvs.recordLimit(level);
    }
    domain.delIdv(idv, level);
    return size() == 0;
}


// Assign methods
void Variable::assignToIdv(int idv, int level) {
    if(domain.idvs.isLimitRecordedAtLevel(level) == false) {
        addToTrail = true;
        domain.idvs.recordLimit(level);
    }
    domain.reduceTo(idv, level);
}


void Variable::assignToVal(int v, int level) {
    if(domain.idvs.isLimitRecordedAtLevel(level) == false) {
        addToTrail = true;
        domain.idvs.recordLimit(level);
    }
    domain.reduceTo(domain.toIdv(v), level);
}


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
