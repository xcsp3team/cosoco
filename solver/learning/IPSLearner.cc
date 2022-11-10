#include "IPSLearner.h"

#include <solver/Solver.h>

#include "mtl/Vec.h"

using namespace Cosoco;


IPSLearner::IPSLearner(Solver &s, Problem &p) : solver(s), problem(p) {
    justifications.growTo(problem.nbVariables());
    selectedVariables.growTo(problem.nbVariables());
    eliminateEntailedVariables      = true;
    eliminateInitialDomainVariables = true;
    eliminateDegreeVariables        = false;

    for(int i = 0; i < justifications.size(); i++) {
        justifications[i].growTo(problem.variables[i]->domain.maxSize(), nullptr);
        for(int j = problem.variables[i]->size(); j < problem.variables[i]->domain.maxSize(); j++)
            justifications[i][j] = rootJustification;
    }
}


void IPSLearner::notifyDomainReduction(Cosoco::Variable *x, int idv, Cosoco::Solver &s) {
    assert(x->containsIdv(idv));   // event is called before value deletion
    justifications[x->idx][idv] = s.decisionLevel() == 0 ? rootJustification : s.currentFilteredConstraint;
}


void IPSLearner::notifyDomainAssignment(Cosoco::Variable *x, int idv, Cosoco::Solver &s) {
    assert(x->containsIdv(idv));   // event is called before value assignment
    for(int i = 0; i < x->size(); i++)
        if(i != idv)
            justifications[x->idx][i] = s.decisionLevel() == 0 ? rootJustification : s.currentFilteredConstraint;
}


void IPSLearner::notifyConflict(Constraint *c, int level) { selectedVariables.fill(false); }


bool IPSLearner::canEliminateDeducedVariable(Variable *x) {
    for(int idv = x->domain.size(); idv < x->domain.maxSize(); idv++) {
        Constraint *explanation = justifications[x->idx][idv];
        if(explanation == nullptr)
            return false;
        if(explanation == rootJustification)
            continue;
        for(Variable *y : explanation->scope)
            if(x != y && selectedVariables[y->idx] == false)
                return false;
    }
    return true;
}


int IPSLearner::getNbFreeVariables(Constraint *c) {
    int nb = 0;
    for(int posx : c->unassignedVariablesIdx)
        if(c->scope[posx]->size() != 1)
            nb++;
    return nb;
}


bool IPSLearner::canEliminateSingletonVariable(Variable *x) {
    assert(x->size() == 1);
    if(solver.isGACGuaranted() == false)
        return false;
    if(problem.isBinary)
        return true;
    for(Constraint *c : x->constraints)
        if(getNbFreeVariables(c) > 1)
            return false;
    return true;
}


bool IPSLearner::canEliminateDegree(Variable *x) { return false; }


bool IPSLearner::canEliminateVariable(Variable *x) {
    if(eliminateEntailedVariables && x->size() == 1 && canEliminateSingletonVariable(x))
        return true;

    if(eliminateInitialDomainVariables && x->size() == x->domain.maxSize())
        return true;

    if(eliminateDegreeVariables && canEliminateDegree(x))
        return true;

    return false;
}


void IPSLearner::extract() {
    selectedVariables.fill(false);
    extracted.clear();

    if(problem.isBinary) {
        for(int i = 0; i < solver.unassignedVariables.size(); i++) {
            Variable *x = solver.unassignedVariables[i];
            if(canEliminateVariable(x) == false) {
                extracted.push(x);
                selectedVariables[x->idx] = true;
            }
        }
    } else {
        for(Variable *x : problem.variables)
            if(canEliminateVariable(x) == false) {
                extracted.push(x);
                selectedVariables[x->idx] = true;
            }
    }

    int nbSelectedVariables = 0;
    for(int i = 0; i < extracted.size(); i++)
        if(canEliminateDeducedVariable(extracted[i]) == false)
            extracted[nbSelectedVariables++] = extracted[i];
    extracted.cut(nbSelectedVariables);
}