#include "SatWrapper.h"

#include <solver/Solver.h>

using namespace Cosoco;
Constraint *SatWrapper::fake = (Constraint *)0x1;


// TODO : be sure no sat unt clause
//
SatWrapper::SatWrapper(Solver *csp) : cspSolver(csp) {
    assert(cspSolver->problem.isConstructionDone);
    satSolver = new Glucose::Solver();
    cspVariableIsBoolean.growTo(cspSolver->problem.nbVariables(), false);
    firstSATVariable.growTo(cspSolver->problem.nbVariables());
    for(Variable *x : cspSolver->problem.variables) createSATEntriesFor(x);
    verbose.log(NORMAL, "c SAT : Variables: %d -- Clauses: %d\n", satSolver->nVars(), satSolver->nClauses());
    cspSolver->addObserverDeleteDecision(this);
    cspSolver->addObserverNewDecision(this);
    cspSolver->addObserverDomainReduction(this);
}


void SatWrapper::startCSPPropagatations() {
    toPropagateInCSP.clear();
    toPropagateInSAT.clear();
}


void SatWrapper::notifyDomainReduction(Variable *x, int idv, Solver &s) { toPropagateInSAT.push(X_ne_idv__ToLiteral(x, idv)); }


void SatWrapper::notifyDomainAssignment(Variable *x, int idv, Solver &s) { toPropagateInSAT.push(X_eq_idv__ToLiteral(x, idv)); }


static int  _lvl;
static bool donotforgetassertivelit;


int SatWrapper::doSATPropagations() {
    int currentSATStack     = donotforgetassertivelit ? satSolver->trail.size() - 1 : satSolver->trail.size();
    donotforgetassertivelit = false;
    Glucose::vec<Glucose::Lit> selectors;   // useless (parameter)
    learntClause.clear();
    bool newProps = true;
    while(newProps) {
        newProps = false;
        Glucose::CRef confl;
        if((confl = satSolver->propagate()) != Glucose::CRef_Undef) {
            learntClause.clear();
            int          backtrackLevel;
            unsigned int nblevels, szWithoutSelectors;

            satSolver->analyze(confl, learntClause, selectors, backtrackLevel, nblevels, szWithoutSelectors);
            if(learntClause.size() > 1) {
                crefLearntClause = satSolver->ca.alloc(learntClause, true);
                satSolver->ca[crefLearntClause].setOneWatched(false);
                satSolver->learnts.push(crefLearntClause);
                satSolver->attachClause(crefLearntClause);
            } else
                crefLearntClause = Glucose::CRef_Undef;
            cspSolver->statistics[SATLearnts]++;
            std::cout << "conflict: sz " << learntClause.size() << "lvl : " << cspSolver->decisionLevel() << " bt"
                      << cspLevel[backtrackLevel];
            satSolver->printLit(learntClause[0]);
            std::cout << "X" << crefLearntClause << std::endl;
            _lvl               = backtrackLevel;
            propagateAssertive = true;
            return cspLevel[backtrackLevel];
        }
        if(toPropagateInSAT.size() == 0)
            break;
        while(toPropagateInSAT.size() > 0) {
            Glucose::Lit lit = toPropagateInSAT.last();
            toPropagateInSAT.pop();
            if(satSolver->value(lit) == l_False)
                return cspSolver->decisionLevel();   // Can not analyze this conflict, backtrack to previous level
            if(satSolver->value(lit) == l_Undef) {
                addNewLevel(lit);
                newProps = true;
                break;
            }
        }
    }
    for(int i = currentSATStack; i < satSolver->trail.size(); i++) {
        assert(satSolver->value(satSolver->trail[i]) != l_Undef);
        if(satSolver->reason(var(satSolver->trail[i])) == Glucose::CRef_Undef ||
           satSolver->ca[satSolver->reason(var(satSolver->trail[i]))].learnt() != true)
            continue;
        toPropagateInCSP.push();
        lit2csp(satSolver->trail[i], toPropagateInCSP.last());
        // std::cout << cspSolver->decisionLevel() << " : " << toPropagateInCSP.last().x->name()
        //          << (toPropagateInCSP.last().equal == true ? "=" : "!=") << toPropagateInCSP.last().idv << std::endl;
    }
    return SATISOK;
}


SatWrapper::SatPropState SatWrapper::addAssertiveLiteralInQueue() {
    // return USELESS;
    if(propagateAssertive && _lvl == satSolver->decisionLevel()) {
        std::cout << " -> ";
        satSolver->printLit(learntClause[0]);
        if(satSolver->value(learntClause[0]) == l_Undef)
            return CONFLICT;
        if(satSolver->value(learntClause[0]) == l_True)
            return USELESS;

        if(satSolver->decisionLevel() == 0)
            satSolver->uncheckedEnqueue(learntClause[0]);
        else
            satSolver->uncheckedEnqueue(learntClause[0], crefLearntClause);
        _lvl                    = -1;
        donotforgetassertivelit = true;
        return PROPAGATE;
    }
    _lvl                    = -1;
    donotforgetassertivelit = false;
    return USELESS;
}


bool SatWrapper::providesNewSATPropagations() {
    bool ok;
    for(CSPPropagation p : toPropagateInCSP) {
        if(p.equal)
            ok = cspSolver->assignToIdv(p.x, p.idv);
        else
            ok = cspSolver->delIdv(p.x, p.idv);
        if(ok == false)
            return false;
    }
    return true;
}


void SatWrapper::addClause(vec<CSPPropagation> &nogood, bool isLearnt) {
    // We can add clauses at level 0 !
    assert(satSolver->decisionLevel() == 0);
    int currentSATStack = satSolver->trail.size();

    Glucose::vec<Glucose::Lit> clause;
    for(CSPPropagation p : nogood) clause.push(p.equal ? X_eq_idv__ToLiteral(p.x, p.idv) : X_ne_idv__ToLiteral(p.x, p.idv));
    satSolver->addClause(clause, isLearnt);
    for(int i = currentSATStack; i < satSolver->trail.size(); i++) {
        toPropagateInCSP.push();
        lit2csp(satSolver->trail[i], toPropagateInCSP.last());
    }
}


void SatWrapper::addNewLevel(Glucose::Lit l) {
    _lvl = -1;
    if(satSolver->value(l) == l_True)
        return;
    assert(satSolver->value(l) == l_Undef);
    if(cspSolver->decisionLevel() > 0) {
        satSolver->newDecisionLevel();
        cspLevel.push(cspSolver->decisionLevel());
    }
    satSolver->uncheckedEnqueue(l);
}


void SatWrapper::notifyNewDecision(Variable *x, Solver &s) {
    if(satSolver->value(X_eq_idv__ToLiteral(x, x->valueId())) == l_False) {
        satSolver->printLit(X_eq_idv__ToLiteral(x, x->valueId()));
        std::cout << cspSolver->decisionLevel() << " " << cspLevel.size() << " " << satSolver->decisionLevel() << std::endl;
        assert(false);
    }
    addNewLevel(X_eq_idv__ToLiteral(x, x->valueId()));
    // std::cout << "+SAT "<< cspLevel.size() <<" " <<satSolver->decisionLevel()
    //          << "CSP " <<(cspLevel.size()==0 ? -1:cspLevel.last()) << " " << cspSolver->decisionLevel()<<std::endl;
    assert(satSolver->decisionLevel() == cspLevel.size());
    // assert(cspLevel.size() == 0 || cspSolver->decisionLevel()==cspLevel.last());
}


void SatWrapper::fullBacktrack() {
    cspLevel.clear();
    satSolver->cancelUntil(0);
}


void SatWrapper::notifyDeleteDecision(Variable *x, int v, Solver &s) {
    donotforgetassertivelit = false;
    while(cspLevel.size() > 0 && cspLevel.last() != cspSolver->decisionLevel()) {
        cspLevel.pop();
        satSolver->cancelUntil(satSolver->decisionLevel() - 1);
    }
    // std::cout << "-SAT "<< cspLevel.size() <<" " <<satSolver->decisionLevel()
    //          << "CSP " <<(cspLevel.size()==0 ? -1:cspLevel.last()) << " " << cspSolver->decisionLevel()<<std::endl;
    assert(satSolver->decisionLevel() == cspLevel.size());
    // assert(cspLevel.size() == 0 || cspSolver->decisionLevel()==cspLevel.last());
}


Glucose::Lit SatWrapper::X_eq_idv__ToLiteral(Variable *x, int idv) {
    return Glucose::mkLit(firstSATVariable[x->idx] + idv, false);
}


Glucose::Lit SatWrapper::X_ne_idv__ToLiteral(Variable *x, int idv) {
    return Glucose::mkLit(firstSATVariable[x->idx] + idv, true);
}


void SatWrapper::lit2csp(Glucose::Lit lit, CSPPropagation &cspProp) {
    int       v   = Glucose::var(lit);
    Variable *x   = satVar2cspVar[v];
    cspProp.x     = x;
    cspProp.equal = Glucose::sign(lit) == false;
    cspProp.idv   = v - firstSATVariable[x->idx];
    assert(cspProp.idv >= 0);
    assert(cspProp.idv < cspSolver->problem.variables[x->idx]->domain.maxSize());
    assert(lit == (cspProp.equal ? X_eq_idv__ToLiteral(cspProp.x, cspProp.idv) : X_ne_idv__ToLiteral(cspProp.x, cspProp.idv)));
}


void SatWrapper::createSATEntriesFor(Cosoco::Variable *x) {
    int nbSATVariables       = satSolver->nVars();
    firstSATVariable[x->idx] = nbSATVariables;   // We set the first one
    if(0 && x->domain.maxSize() == 2) {          // TODO boolean var
        satSolver->newVar();
        cspVariableIsBoolean[x->idx] = true;
        return;
    }
    for(int i = 0; i < x->domain.maxSize(); i++) {
        satSolver->newVar();
        satVar2cspVar.push(x);
    }

    // Create atleast clauses
    Glucose::vec<Glucose::Lit> clause;
    for(int i = 0; i < x->domain.maxSize(); i++) clause.push(X_eq_idv__ToLiteral(x, i));
    satSolver->addClause(clause);

    // At most clause
    for(int i = 0; i < x->domain.maxSize(); i++)
        for(int j = i + 1; j < x->domain.maxSize(); j++) {
            clause.clear();
            clause.push(X_ne_idv__ToLiteral(x, i));
            clause.push(X_ne_idv__ToLiteral(x, j));
            satSolver->addClause(clause);
        }
}
