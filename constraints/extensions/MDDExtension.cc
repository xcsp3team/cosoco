#include "MDDExtension.h"

#ifdef USE_XCSP3

#include "constraints/extensions/structures/MDD.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool MDDExtension::isSatisfiedBy(vec<int> &tuple) {
    MDDNode *current = mdd->root;
    for(int i = 0; i < tuple.size(); i++) {
        int idv = scope[i]->domain.toIdv(tuple[i]);
        assert(idv >= 0 && idv < current->childs.size());
        if(current->childs[idv] == falseNode)
            return false;
        current = current->childs[idv];
    }
    assert(current = mdd->trueNode);
    return true;
}


bool MDDExtension::isCorrectlyDefined() { return true; }


//----------------------------------------------
// Main filtering method
//----------------------------------------------
bool MDDExtension::filter(Variable *x) {
    beforeFiltering();
    exploreMDD(mdd->root);
    return updateDomains();
}


void MDDExtension::beforeFiltering() {
    nbTotalValuesWithoutSupports = 0;

    for(int pos = 0; pos < scope.size(); pos++) {
        int domSize = scope[pos]->size();
        nbTotalValuesWithoutSupports += domSize;
        nbValuesWithoutSupports[pos] = domSize;
        gac.fillRow(pos, false);
    }
    trueTimestamp++;
}


bool MDDExtension::exploreMDD(MDDNode *node) {
    if(node == falseNode || (falseNodes[node->id] == falseTimestamp))
        return false;

    if(node == mdd->trueNode || trueNodes[node->id] == trueTimestamp)
        return true;

    Variable *x         = scope[node->level];
    bool      supported = false;
    if(x->size() < node->nbChilds()) {
        for(int idv : x->domain) {
            if(node->childs[idv] == falseNode)
                continue;
            if(exploreMDD(node->childs[idv])) {
                supported = true;
                manageSuccessfulExploration(x, idv, node->level);
            }
        }
    } else {
        for(int idv : node->directAccessToChilds) {
            if(x->containsIdv(idv) && exploreMDD(node->childs[idv])) {
                supported = true;
                manageSuccessfulExploration(x, idv, node->level);
            }
        }
    }

    if(supported)
        trueNodes[node->id] = trueTimestamp;
    else
        falseNodes[node->id] = falseTimestamp;
    return supported;
}


bool MDDExtension::updateDomains() {
    for(int pos = 0; pos < scope.size(); pos++) {
        Variable *x          = scope[pos];
        int       nbRemovals = nbValuesWithoutSupports[pos];
        if(nbRemovals == 0)
            continue;
        int nb = 0;
        for(int idv : reverse(x->domain)) {
            if(gac[pos][idv] == false) {
                if(solver->delIdv(x, idv) == false)
                    return false;
                if(++nb == nbRemovals)
                    break;
            }
        }
        if((nbTotalValuesWithoutSupports -= nbRemovals) == 0)
            break;
    }
    return true;
}


bool MDDExtension::manageSuccessfulExploration(Variable *x, int idv, int level) {
    // assert(toScopePosition(x->idx) == level);
    if(!gac[level][idv]) {
        nbTotalValuesWithoutSupports--;
        nbValuesWithoutSupports[level]--;
        gac[level][idv] = true;
    }
    return false;
}

//----------------------------------------------
// Constructors and initialisation
//----------------------------------------------

MDDExtension::MDDExtension(Problem &p, std::string n, vec<Variable *> &vars, vec<XCSP3Core::XTransition *> &transitions)
    : MDDExtension(p, n, vars, new MDD(transitions, vars)) { }


MDDExtension::MDDExtension(Problem &p, std::string n, vec<Variable *> &vars, MDD *m)
    : Extension(p, n, vars, 0, true), mdd(m), gac(Matrix<bool>()) {
    type          = "MDD";
    trueTimestamp = falseTimestamp = 1;
    nbValuesWithoutSupports.growTo(vars.size());
    trueNodes.growTo(mdd->nbNodes);
    falseNodes.growTo(mdd->nbNodes);
    int maxSize = vars[0]->domain.maxSize();
    for(Variable *x : vars)
        if(x->domain.maxSize() > maxSize)
            maxSize = x->domain.maxSize();
    gac.initialize(vars.size(), maxSize);
    gac.growTo(vars.size());
}


void MDDExtension::attachSolver(Solver *s) {
    Constraint::attachSolver(s);
    s->addObserverDeleteDecision(this);
}


void MDDExtension::notifyDeleteDecision(Variable *x, int v, Solver &s) { falseTimestamp++; }

#endif /* USE_XCSP3 */
