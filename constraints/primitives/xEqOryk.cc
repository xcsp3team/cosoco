//
// Created by audemard on 29/01/2021.
//

#include "xEqOryk.h"

#include "solver/Solver.h"

using namespace Cosoco;
//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool xEqGenOr::isSatisfiedBy(vec<int> &tuple) {
    return true;
    int r            = tuple.last();
    int clauseResult = 0;
    int i            = 0;
    for(BasicNode *n : nodes) {
        if(n->value())
            clauseResult = 1;
    }

    return r == clauseResult;
}

bool xEqGenAnd::isSatisfiedBy(vec<int> &tuple) {
    return true;
    int r            = tuple.last();
    int clauseResult = 0;
    int i            = 0;
    for(BasicNode *n : nodes) {
        if(n->value())
            clauseResult = 1;
    }

    return r == clauseResult;
}

bool GenOr::isSatisfiedBy(vec<int> &tuple) {
    return true;
    int r            = tuple.last();
    int clauseResult = 0;
    int i            = 0;
    for(BasicNode *n : nodes) {
        if(n->value())
            clauseResult = 1;
    }

    return r == clauseResult;
}
//----------------------------------------------
// Filtering
//----------------------------------------------

bool xEqGenOr::filter(Variable *x) {
    // useless ??
    if(solver->decisionLevel() == 0 && (result->size() > 2 || result->minimum() != 0 || result->maximum() != 1)) {
        for(int idv : result->domain) {
            int v = result->domain.toVal(idv);
            if(v != 0 && v != 1 && solver->delVal(x, idv) == false)
                return false;
        }
    }

    if(result->size() == 1) {
        if(result->value() == 0) {
            for(BasicNode *n : nodes)
                if(n->setFalse(solver) == false)
                    return false;
            solver->entail(this);
            return true;
        }
        assert(result->value() == 1);
        int nb  = 0;
        int pos = -1;
        int i   = 0;
        for(BasicNode *n : nodes) {
            if(n->minimum() == 1) {
                solver->entail(this);
                return true;
            }
            if(n->maximum() == 1) {
                nb++;
                if(pos == -1)
                    pos = i;
                else
                    break;
            }
            i++;
        }
        if(nb == 0)
            return false;
        if(nb == 1) {
            nodes[pos]->setTrue(solver);
            return solver->entail(this);
        }
    }

    int nb = 0;
    for(BasicNode *n : nodes) {
        if(n->minimum() == 1) {
            solver->assignToVal(result, 1);
            return solver->entail(this);
        }
        if(n->maximum() == 1)
            nb++;
    }
    if(nb == 0) {
        solver->assignToVal(result, 0);
        return solver->entail(this);
    }
    return true;
}


bool xEqGenAnd::filter(Variable *x) {
    // useless ??
    if(solver->decisionLevel() == 0 && (result->size() > 2 || result->minimum() != 0 || result->maximum() != 1)) {
        for(int idv : result->domain) {
            int v = result->domain.toVal(idv);
            if(v != 0 && v != 1 && solver->delVal(x, idv) == false)
                return false;
        }
    }

    if(result->size() == 1) {
        if(result->value() == 1) {
            for(BasicNode *n : nodes)
                if(n->setTrue(solver) == false)
                    return false;
            solver->entail(this);
            return true;
        }
        assert(result->value() == 0);
        int nb  = 0;
        int pos = -1;
        int i   = 0;
        for(BasicNode *n : nodes) {
            if(n->maximum() == 0) {
                solver->entail(this);
                return true;
            }
            if(n->minimum() == 0) {
                nb++;
                if(pos == -1)
                    pos = i;
                else
                    break;
            }
            i++;
        }
        if(nb == 0)
            return false;
        if(nb == 1) {
            nodes[pos]->setFalse(solver);
            return solver->entail(this);
        }
    }

    int nb = 0;
    for(BasicNode *n : nodes) {
        if(n->maximum() == 0) {
            solver->assignToVal(result, 0);
            return solver->entail(this);
        }
        if(n->minimum() == 0)
            nb++;
    }
    if(nb == 0) {
        solver->assignToVal(result, 1);
        return solver->entail(this);
    }
    return true;
}


int GenOr::findSentinel(int other) {
    for(int i = 0; i < nodes.size(); i++)
        if(i != other && nodes[i]->maximum() == 1)
            return i;
    return -1;
}

bool GenOr::filter(Variable *x) {
    if(nodes[s1]->maximum() == 0) {
        int o = findSentinel(s2);
        if(o == -1 && nodes[s2]->setTrue(solver) == false)
            return false;
        return solver->entail(this);
    }
    if(nodes[s2]->maximum() == 0) {
        int o = findSentinel(s1);
        if(o == -1 && nodes[s1]->setTrue(solver) == false)
            return false;
        return solver->entail(this);
    }
    return true;
}

//----------------------------------------------
// Constructor and initialisation methods
//----------------------------------------------

xEqGenOr::xEqGenOr(Problem &p, std::string n, Variable *r, vec<Variable *> &vars, vec<BasicNode *> &nnodes)
    : GlobalConstraint(p, n, "X = Generalized OR", Constraint::createScopeVec(&vars, r)), result(r) {
    nnodes.copyTo(nodes);
}


xEqGenAnd::xEqGenAnd(Problem &p, std::string n, Variable *r, vec<Variable *> &vars, vec<BasicNode *> &nnodes)
    : GlobalConstraint(p, n, "X = Generalized AND", Constraint::createScopeVec(&vars, r)), result(r) {
    nnodes.copyTo(nodes);
}

GenOr::GenOr(Problem &p, std::string n, vec<Variable *> &vars, vec<BasicNode *> &nnodes)
    : GlobalConstraint(p, n, "Generalized OR", Constraint::createScopeVec(&vars)) {
    nnodes.copyTo(nodes);
    s1 = 0;
    s2 = 1;
}