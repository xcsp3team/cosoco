#include "./Solver.h"

#include <solver/heuristics/values/HeuristicValLast.h>
#include <solver/heuristics/values/HeuristicValRandom.h>
#include <solver/heuristics/variables/HeuristicVarCACD.h>
#include <solver/heuristics/variables/HeuristicVarFirst.h>
#include <solver/restarts/GeometricRestart.h>
#include <solver/restarts/InnerOuter.h>
#include <solver/restarts/LubyRestart.h>
#include <utils/System.h>

#include <fstream>
#include <iomanip>

#include "HeuristicValASGS.h"
#include "HeuristicValOccs.h"
#include "HeuristicValRoundRobin.h"
#include "HeuristicVarFRBA.h"
#include "HeuristicVarRoundRobin.h"
#include "Options.h"
#include "PickOnDom.h"
#include "PoolOfHeuristicsValues.h"
#include "constraints/Constraint.h"
#include "core/Variable.h"
#include "heuristics/values/HeuristicValFirst.h"
#include "heuristics/values/HeuristicValStickingValue.h"
#include "heuristics/variables/HeuristicVarDomWdeg.h"
#include "heuristics/variables/LastConflictReasoning.h"
#include "heuristics/variables/RandomizeFirstDescent.h"
using namespace Cosoco;
using namespace std;
//----------------------------------------------
// Constructor / Initialisation
//----------------------------------------------


Solver::Solver(Problem &p)
    : AbstractSolver(p),
      problem(p),
      unassignedVariables(p.nbVariables(), p.variables, true),
      decisionVariables(p.nbVariables(), p.variables, true),
      entailedConstraints(p.nbConstraints(), false),
      queue(p.nbVariables(), p.variables) {
    heuristicVar = nullptr;
    heuristicVal = nullptr;
    statistics.growTo(NBSTATS, 0);
    localstatistics.growTo(NBLOCALSTATS, 0);
    problem.attachSolver(this);
    lastSolutionRun     = 0;
    nbSolutions         = 0;
    warmStart           = false;
    nogoodsFromRestarts = false;
    displaySolution     = options::boolOptions["model"].value;

    if(options::boolOptions["profile"].value)
        profiling = new Profiling(this);
    doProfiling = options::boolOptions["profile"].value;

    if(options::stringOptions["val"].value == "first")
        heuristicVal = new HeuristicValFirst(*this);
    if(options::stringOptions["val"].value == "rand")
        heuristicVal = new HeuristicValRandom(*this);
    if(options::stringOptions["val"].value == "last")
        heuristicVal = new HeuristicValLast(*this);
    if(options::stringOptions["val"].value == "robin")
        heuristicVal = new HeuristicValRoundRobin(*this, options::stringOptions["robin"].value);
    if(options::stringOptions["val"].value == "occs")
        heuristicVal = new HeuristicValOccs(*this);
    if(options::stringOptions["val"].value == "asgs")
        heuristicVal = new HeuristicValASGS(*this);
    if(options::stringOptions["val"].value == "pool")
        heuristicVal = new PoolOfHeuristicsValues(*this);
    if(heuristicVal == nullptr) {
        std::cerr << "unknown heuristic value " << options::stringOptions["val"].value << "\n";
        exit(1);
    }


    if(options::stringOptions["var"].value == "wdeg")
        heuristicVar = new HeuristicVarDomWdeg(*this);
    if(options::stringOptions["var"].value == "cacd")
        heuristicVar = new HeuristicVarCACD(*this);
    if(options::stringOptions["var"].value == "pick")
        heuristicVar = new PickOnDom(*this);
    if(options::stringOptions["var"].value == "frba")
        heuristicVar = new HeuristicVarFRBA(*this);
    if(options::stringOptions["var"].value == "robin")
        heuristicVar = new HeuristicVarRoundRobin(*this);
    if(heuristicVar == nullptr) {
        std::cerr << "unknown heuristic variable " << options::stringOptions["var"].value << "\n";
        exit(1);
    }


    /*if(options::stringOptions["warmstart" != "") {
        std::ifstream warmFile(warmStart);
        vec<int>      values;
        for(std::string line; std::getline(warmFile, line);) {
            std::vector<std::string> stringValues = split1(line, ' ');
            for(const std::string &v : stringValues) {
                if(v.find("x") != std::string::npos) {
                    std::vector<std::string> compact = split1(v, 'x');
                    int                      tmp     = compact[0].find("*") != std::string::npos ? STAR : std::stoi(compact[0]);
                    for(int i = 0; i < std::stoi(compact[1]); i++) values.push(tmp);
                } else {
                    if(v.find("*") != std::string::npos) {
                        values.push(STAR);
                    } else {
                        values.push(std::stoi(v));
                    }
                }
            }
        }
        S->warmStart    = true;
        S->heuristicVal = new ForceIdvs(*S, S->heuristicVal, false, &values);
    }*/

    if(options::intOptions["lc"].value > 0)
        addLastConflictReasoning();
    if(options::boolOptions["stick"].value)
        addStickingValue();
    if(options::stringOptions["restarts"].value == "geo")
        restart = new GeometricRestart(this);
    if(options::stringOptions["restarts"].value == "luby")
        restart = new LubyRestart(this);
    if(options::stringOptions["restarts"].value == "io")
        restart = new InnerOuterRestart(this);
    if(restart == nullptr && options::stringOptions["restarts"].value != "no") {
        std::cerr << "unknown heuristic restart " << options::stringOptions["restart"].value << "\n";
        exit(1);
    }


    if(options::boolOptions["nogoods"].value)
        addNoGoodsFromRestarts();

    /*if(annotations && cb.decisionVariables[core].size() != 0)
        S->setDecisionVariables(cb.decisionVariables[core]);
    else {
        if(0 && cb.nbInitialsVariables != solvingProblems[0]->nbVariables()) {
            vec<Variable *> tmp;
            for(int i = 0; i < cb.nbInitialsVariables; i++) tmp.push(solvingProblems[0]->variables[i]);
            S->setDecisionVariables(tmp);
            printf("%d\n", tmp.size());
            if(core == 0)
                S->verbose.log(NORMAL, "c Limit decision variables to initial ones (%d vars)\n", cb.nbInitialsVariables);
        }
    }*/
}

void Solver::addNoGoodsFromRestarts() {
    nogoodsFromRestarts = true;
    noGoodsEngine       = new NoGoodsEngine(*this);
}

void Solver::addLastConflictReasoning() {
    heuristicVar = new LastConflictReasoning(*this, heuristicVar, options::intOptions["lc"].value);
}


void Solver::addRandomizationFirstDescent() { heuristicVar = new RandomizeFirstDescent(*this, heuristicVar); }


void Solver::addStickingValue() { heuristicVal = new HeuristicValStickingValue(*this, heuristicVal); }


void Solver::setDecisionVariables(vec<Variable *> &vars) {
    if(vars.size() == 0)
        return;
    decisionVariables.clear();
    for(Variable *v : vars) decisionVariables.add(v);
}

//----------------------------------------------
// Search methods
//----------------------------------------------

int Solver::solve(vec<RootPropagation> &assumps) {
    bool firstCall = nbSolutions == 0;

    if(doProfiling && firstCall)
        profiling->initialize();


    fullBacktrack();

    if(problem.isConstructionDone == false)
        throw std::logic_error("c\nc Error: the construction problem is not finished...! (problem.delayedConstruction())\nc\n");

    if(nogoodsFromRestarts && restart == nullptr)
        verbose.log(NORMAL, "c WARNING: no restarts, but nogoods from restarts are enabled\n");

    lastSolution.clear();   // Store the last solution
    entailedConstraints.clear();
    status         = RUNNING;
    nbSolutions    = 0;
    Constraint *pc = propagateComplete();
    if(pc != nullptr) {
        status = FULL_EXPLORATION;
        return R_UNSAT;
    }
    if(firstCall) {
        verbose.log(NORMAL, "c root propagations     : %-12lld\n", statistics[rootPropagations]);
        if(restart == nullptr)
            verbose.log(NORMAL, "c restarts disabled, print stats every %d conflicts\n", displayStatsEveryConflicts);
        else
            verbose.log(NORMAL, "c restarts enabled, print stats at every restarts\n");
        verbose.log(NORMAL, "c\nc\n");
        displayHeaderCurrentSearchSpace();
    }

    // Remove useless variables:
    for(Variable *x : problem.variables)
        if(x->useless) {
            unassignedVariables.del(x);
            decisionVariables.del(x);
        }

    return search(assumps);
}


int Solver::search(vec<RootPropagation> &assumptions) {
    while(status == RUNNING) {
        if(propagate() != nullptr) {   // A conflict occurs
            if(stopSearch)
                return R_UNKNOWN;
            conflicts++;
            updateStatisticsWithNewConflict();
            handleFailure(decisionLevel());
            if(status == FULL_EXPLORATION)
                break;
            if(restart != nullptr && restart->isItTimeToRestart())   // Manage restart
                doRestart();
        } else {
            if(heuristicVar->stop())   // Only useful if LNS is used in optimizer: stop the search with the fragment
                break;
            if(decisionVariables.isEmpty()) {   // A solution is found
                if(manageSolution())            // The search is finished
                    break;
                handleFailure(decisionLevel());   // remove the last decision level
            } else {
                // Start with assumptions
                Variable *x   = nullptr;
                int       idv = -1;

                // Perform user provided assumption:
                while(decisionLevel() < assumptions.size()) {
                    // printf("Decision level %d\n", decisionLevel());
                    RootPropagation &assumption = assumptions[decisionLevel()];
                    // printf("x=%d equal=%d idv=%d\n", assumption.idx,assumption.equal, assumption.idv);
                    // assert(assumption.equal); // We deal only with positive assumptions
                    Variable *tmp = problem.variables[assumption.idx];

                    if(tmp->containsIdv(assumption.idv) == false) {   // UNSAT WRT Assumptions
                        fullBacktrack();
                        return R_UNSAT;
                    } else {
                        x   = tmp;
                        idv = assumption.idv;
                        break;
                    }
                }

                if(x == nullptr) {                  // No assumptions
                    x   = heuristicVar->select();   // A new decision variable
                    idv = heuristicVal->select(x);
                }
                newDecision(x, idv);
            }
        }
    }

    if(nbSolutions > 0)
        return R_SAT;
    if(nbSolutions == 0)
        return R_UNSAT;


    return R_UNKNOWN;
}


bool Solver::manageSolution() {
    nbSolutions++;
    lastSolutionRun = statistics[restarts];
#ifdef COMPARESOLUTIONS
    for(vec<int> &sol : allSolutions) {
        bool ok = true;
        for(int i = 0; i < sol.size(); i++)
            if(problem.variables[i]->useless == false && sol[i] != problem.variables[i]->value())
                ok = false;
        if(ok) {
            std::cout << "Same solution" << std::endl;
            exit(1);
        }
    }

    allSolutions.push();

    for(Variable *x : problem.variables) allSolutions.last().push(x->useless ? 0 : x->value());
#endif

    if(checkSolution)
        problem.checkSolution();
    lastSolution.clear();   // Store the last solution

    for(Variable *x : problem.variables) lastSolution.push(x->useless ? 0 : x->value());

    if(displaySolution)
        displayCurrentSolution();

    if(nbSolutions > 1 || nbSolutions == 0)   // Add nogood
        noGoodsEngine->generateNogoodFromSolution();

    if(nbSolutions == options::intOptions["nbsols"].value) {
        status = REACH_GOAL;
        return true;
    }
    return false;
}


//----------------------------------------------
// Decision methods
//----------------------------------------------

void Solver::newDecision(Variable *x, int idv) {
    decisions++;
    nodes++;
    assert(x->size() >= 1);
    unassignedVariables.del(x);   // Now, x is assigned
    decisionVariables.del(x);
    trail_lim.push(trail.size());   // a new decision level, store the current trail
    decisionVariablesId.push(idv);
    verbose.log(DEBUGVERBOSE, "%slvl %d : %s = %d (initial domain= %d) %s\n", KGRN, decisionLevel(), x->name(),
                x->domain.toVal(idv), x->size(), KNRM);

    assignToIdv(x, idv);
    for(Constraint *c : x->constraints) c->assignVariable(x);
    notifyNewDecision(x);
    x->domain.nAssignments[idv]++;
}


Variable *Solver::decisionVariableAtLevel(int lvl) {
    assert(lvl > 0);
    assert(decisionLevel() >= lvl);
    Variable *x = (trail[trail_lim[lvl - 1]]);
    assert(x->size() <= 1);   // Domain of decision variable can be empty
    assert(unassignedVariables.contains(x) == false);
    return x;
}


int Solver::decisionLevel() const { return trail_lim.size(); }


bool Solver::isAssigned(Variable *x) const { return unassignedVariables.contains(x) == false; }

//----------------------------------------------
// Backtrack methods
//----------------------------------------------

void Solver::reinitializeConstraints() {
    for(Constraint *c : problem.constraints) {   // Reinitialisze status for all constraints
        c->reinitialize();
        assert(c->status() == UNDEF);
    }
}

void Solver::backtrack(int lvl) {
    while(decisionLevel() > lvl) backtrack();
}

void Solver::fullBacktrack(bool all) {
    queue.clear();

    while(decisionLevel() > 0) backtrack();
    if(all) {
        trail.clear();
        queue.clear();   // Useless ?
        trail_lim.clear();
        decisionVariablesId.clear();
        for(Variable *x : problem.variables)   // Reinitialize all domain variables.
            x->domain.reinitialize();
    }
    notifyFullBactrack();
}


void Solver::backtrack() {
    assert(decisionLevel() > 0);
    verbose.log(DEBUGVERBOSE, "%s--- BACKTRACK %d ---%s\n", KRED, decisionLevel() - 1, KNRM);
    if(verbose.verbosity == TOTALVERBOSE)
        displayTrail();
    Variable *assigned = decisionVariableAtLevel(decisionLevel());
    int       lvl      = decisionLevel();
    int       v        = assigned->domain.toVal(decisionVariablesId.last());

    for(int idx = trail.size() - 1; idx >= trail_lim[lvl - 1]; idx--) trail[idx]->domain.restoreLimit(lvl);

    trail.shrink(trail.size() - trail_lim[lvl - 1]);   // Remove bad choices and props
    trail_lim.shrink(1);                               // One level less
    decisionVariablesId.shrink(1);
    queue.clear();   // Clear the propagation queue
    if(entailedConstraints.isLimitRecordedAtLevel(lvl))
        entailedConstraints.restoreLimit(lvl);

    unassignedVariables.add(assigned);   // Unassign decision variable
    decisionVariables.add(assigned);
    for(Constraint *c : assigned->constraints)   // those constraints have to knwon
        c->unassignVariable(assigned);
    if(assigned->size() > 1)
        wrongDecisions++;
    notifyDeleteDecision(assigned, v);
}


void Solver::handleFailure(Variable *x, int idv) {
    Variable *tmp    = x;
    int       idvtmp = idv;
    delIdv(tmp, idvtmp);
    assert(tmp->size() == 0);
    int round = 0;
    while(tmp->size() == 0 || propagate() != nullptr) {
        if(decisionLevel() == 0) {
            status = FULL_EXPLORATION;
            break;
        }
        round++;
        tmp    = decisionVariableAtLevel(decisionLevel());
        idvtmp = decisionVariablesId.last();
        backtrack();
        delIdv(tmp, idvtmp);
        nodes++;
    }
}


void Solver::handleFailure(int level) {
    while(decisionLevel() != level) backtrack();
    if(decisionLevel() == 0) {
        status = FULL_EXPLORATION;
        return;
    }
    handleFailure(decisionVariableAtLevel(decisionLevel()), decisionVariablesId.last());
}


void Solver::doRestart() {
    if(nogoodsFromRestarts && noGoodsEngine->generateNogoodsFromRestarts() == false)
        status = FULL_EXPLORATION;
    statistics[restarts]++;
    fullBacktrack();
    if(verbose.verbosity >= 1)
        displayCurrentSearchSpace();
    reinitializeConstraints();
    if(nogoodsFromRestarts)
        noGoodsEngine->enqueueNoGoodsOfSize1();

    propagate(true);
}

//----------------------------------------------
// Propagation methods
//----------------------------------------------

void Solver::addToQueue(Variable *x) {
    queue.add(x);
    x->timestamp = ++timestamp;
}


Variable *Solver::pickInQueue() {   // Select the variable with the smallest domain
    int       minDomain = INT_MAX;
    Variable *x         = nullptr;
    for(int i = 0; i < queue.size(); i++) {
        int curSize = queue[i]->size();
        if(curSize < minDomain) {
            minDomain = curSize;
            x         = queue[i];
        }
    }
    assert(x != nullptr);
    queue.del(x);
    return x;
}


bool Solver::filterConstraint(Cosoco::Constraint *c, Cosoco::Variable *x) {
    filterCalls++;
    nbDeletedValuesByAVariable = 0;
    currentFilteredConstraint  = c;

    c->timestamp = ++timestamp;

    doProfiling && profiling->beforeConstraintCall(c);
    bool noConflict = c->filterFrom(x);
    doProfiling && profiling->afterConstraintCall(c, nbDeletedValuesByAVariable);

    if(noConflict == false) {   // Inconsistent
        pickVariables.push({x, nbDeletedValuesByAVariable == 0 ? 1 : nbDeletedValuesByAVariable});

        notifyConflict(c);
        currentFilteredConstraint = nullptr;
        return false;
    }
    currentFilteredConstraint = nullptr;
    if(nbDeletedValuesByAVariable == 0)
        statistics[uselessFilterCalls]++;
    else
        pickVariables.push({x, nbDeletedValuesByAVariable});
    c->timestamp = ++timestamp;

    return true;
}

Constraint *Solver::propagate(bool startWithSATEngine) {
    currentFilteredConstraint = nullptr;
    postponeFiltering.clear();   // The filtering starts here
    pickVariables.clear();
    while(true) {
        while(queue.size() > 0) {
            Variable *x = pickInQueue();
            assert(x->size() > 0);

            if(nogoodsFromRestarts && noGoodsEngine->propagate(x) == false)
                return NoGoodsEngine::fake;


            for(Constraint *c : x->constraints) {
                if(c->isDisabled)
                    continue;
                if(x->timestamp > c->timestamp && isEntailed(c) == false) {
                    if(c->postpone()) {   // Postpone the filtering of these constraints
                        postponeFiltering.insert(c);
                        c->postponedBy = x;
                        continue;
                    }


                    if(filterConstraint(c, x) == false)
                        return c;
                }
            }
        }


        // Filter postponed constraints
        for(auto *c : postponeFiltering) {
            // std::cout << "post" << c->type << std::endl;
            if(filterConstraint(c, c->postponedBy) == false)
                return c;
        }
        postponeFiltering.clear();
        if(queue.size() == 0)
            break;
    }
    return nullptr;
}


Constraint *Solver::propagateComplete() {
    assert(queue.isEmpty());
    assert(decisionLevel() == 0);

    queue.fill();
    for(Variable *x : problem.variables) x->timestamp = ++timestamp;

    return propagate();
}


bool Solver::isGACGuaranted() { return false; }

//----------------------------------------------------------
// Variable modifications : assign/remove values from domain
//----------------------------------------------------------

// Use these two methods to delete variable values
// These two methods manage the trail !

bool Solver::delIdv(Variable *x, int idv) {
    if(x->domain.containsIdv(idv) == false)
        return true;

    nbDeletedValuesByAVariable++;

    notifyDomainReduction(x, idv);
    x->delIdv(idv, decisionLevel());

    if(x->addToTrail)
        trail.push(x);
    x->addToTrail = false;
    propagations++;
    if(decisionLevel() == 0)
        statistics[rootPropagations]++;
    verbose.log(FULLVERBOSE, "   lvl %d : %s (|d|=%d) -= {%d}\n", decisionLevel(), x->name(), x->size(), x->domain.toVal(idv));
    addToQueue(x);
    return x->size() != 0;
}


bool Solver::delVal(Variable *x, int v) {
    if(x->containsValue(v) == false)
        return true;
    return delIdv(x, x->domain.toIdv(v));
}


bool Solver::assignToVal(Variable *x, int v) {
    // return assignToIdv(x,x->domain.toIdv(v));
    Domain &d   = x->domain;
    int     idv = x->domain.toIdv(v);
    if(d.containsIdv(idv) == false)
        return false;
    if(d.size() == 1)
        return true;   // already assigned to v

    propagations++;
    if(decisionLevel() == 0)
        statistics[rootPropagations]++;

    nbDeletedValuesByAVariable += x->size() - 1;
    notifyDomainAssignment(x, idv);
    x->assignToIdv(idv, decisionLevel());
    if(x->addToTrail)
        trail.push(x);
    x->addToTrail = false;
    addToQueue(x);
    verbose.log(FULLVERBOSE, "   lvl %d : %s = %d\n", decisionLevel(), x->name(), v);
    return true;
}


bool Solver::assignToIdv(Variable *x, int idv) {
    Domain &d = x->domain;
    if(d.containsIdv(idv) == false)
        return false;
    propagations++;
    if(decisionLevel() == 0)
        statistics[rootPropagations]++;

    // if(d.size()==1) return true;
    nbDeletedValuesByAVariable += x->size() - 1;
    notifyDomainAssignment(x, idv);

    x->assignToIdv(idv, decisionLevel());
    if(x->addToTrail)
        trail.push(x);
    x->addToTrail = false;
    addToQueue(x);
    verbose.log(FULLVERBOSE, "   lvl %d : %s = %d\n", decisionLevel(), x->name(), x->domain.toVal(idv));
    return true;
}
//----------------------------------------------
// Multiple values removal (helpers)
//----------------------------------------------

bool Solver::delValuesGreaterOrEqualThan(Variable *x, int max) {
    if(x->minimum() >= max)
        return false;
    for(int idv : reverse(x->domain)) {   // Reverse traversal because of deletion
        if(x->domain.toVal(idv) < max)
            return true;
        if(delIdv(x, idv) == false)
            return false;
    }
    return true;
}


bool Solver::delValuesLowerOrEqualThan(Variable *x, int min) {
    if(x->maximum() <= min)
        return false;
    for(int idv : (x->domain)) {
        if(x->domain.toVal(idv) > min)
            return true;
        if(delIdv(x, idv) == false)
            return false;
    }
    return true;
}


bool Solver::delValuesInDomain(Variable *x, Domain &d) {
    for(int idv : d)
        if(delVal(x, d.toVal(idv)) == false)
            return false;
    return true;
}


bool Solver::delValuesNotInDomain(Variable *x, Domain &d) {
    for(int idv : reverse(x->domain)) {
        int v = x->domain.toVal(idv);
        if(d.containsValue(v) == false && delIdv(x, idv) == false)
            return false;
    }
    return true;
}


bool Solver::changeDomain(Variable *x, SparseSet &newidvalues) {
    assert(newidvalues.size() > 0);     // Initial nodmain must be equal
    for(int idv : reverse(x->domain))   // Reverse traversal : delete values
        if(newidvalues.contains(idv) == false)
            delIdv(x, idv);
    return true;
}


bool Solver::delValuesInRange(Variable *x, int start, int stop) {
    if(start >= stop)
        return true;
    int first = x->minimum(), last = x->maximum();
    if(start > last || stop < first)
        return true;   // because there is no overlapping
    int left = std::max(start, first), right = std::min(stop - 1, last);
    if(left == first && right == last)
        return false;
    for(int v = left; v <= right; v++)
        if(delVal(x, v) == false)
            return false;
    return true;
}


//----------------------------------------------
// Observers methods
//----------------------------------------------

void Solver::notifyConflict(Constraint *c) {
    for(ObserverConflict *oc : observersConflict)   // Notify listeners
        oc->notifyConflict(c, decisionLevel());
}


void Solver::addObserverConflict(ObserverConflict *oc) { observersConflict.push(oc); }


void Solver::addObserverNewDecision(ObserverNewDecision *od) { observersNewDecision.push(od); }


void Solver::addObserverDeleteDecision(ObserverDeleteDecision *od) { observersDeleteDecision.push(od); }


void Solver::notifyNewDecision(Variable *x) {
    for(ObserverNewDecision *od : observersNewDecision) od->notifyNewDecision(x, *this);
}


void Solver::notifyDeleteDecision(Variable *x, int v) {
    for(ObserverDeleteDecision *od : observersDeleteDecision) od->notifyDeleteDecision(x, v, *this);
}


void Solver::notifyFullBactrack() {
    for(ObserverDeleteDecision *od : observersDeleteDecision) od->notifyFullBacktrack();
}


void Solver::addObserverDomainReduction(ObserverDomainReduction *odr) { observersDomainReduction.push(odr); }


void Solver::notifyDomainReduction(Variable *x, int idv) {
    for(ObserverDomainReduction *odr : observersDomainReduction) odr->notifyDomainReduction(x, idv, *this);
}


void Solver::notifyDomainAssignment(Variable *x, int idv) {
    for(ObserverDomainReduction *odr : observersDomainReduction) odr->notifyDomainAssignment(x, idv, *this);
}

//----------------------------------------------
// Minor methods
//----------------------------------------------


void Solver::updateStatisticsWithNewConflict() {
    if(localstatistics[minDepth] == 0 || localstatistics[minDepth] > decisionLevel())
        localstatistics[minDepth] = decisionLevel();

    if(localstatistics[maxDepth] < decisionLevel())
        localstatistics[maxDepth] = decisionLevel();
    localstatistics[sumDepth] += decisionLevel();
    localstatistics[nbConflicts]++;
    if(restart == nullptr && conflicts % displayStatsEveryConflicts == 0)
        displayCurrentSearchSpace();
}


void Solver::displayCurrentSolution() {
    printf("c solution %d\n", nbSolutions);
    printf("\nv <instantiation type='solution'>\n");
    printf("v <list>");
    for(Variable *x : problem.variables)
        if(x->_name.rfind("__av", 0) != 0)
            printf("%s ", x->name());
    printf("</list>\n");
    printf("v <values>");
    for(Variable *x : problem.variables)
        if(x->_name.rfind("__av", 0) != 0) {
            if(x->useless)
                printf("* ");
            else
                printf("%d ", lastSolution[x->idx]);
        }
    printf("</values>\n");

    printf("v </instantiation>\n\n");
    std::cout << std::flush;
}


void Solver::displayTrail() {   // DEBUG method
    fprintf(stderr, "Trail : ");
    int tmp = 0;
    for(int i = 0; i < trail.size(); i++) {
        if(i == trail_lim[tmp]) {
            fprintf(stderr, " | ");
            tmp++;
        }
        fprintf(stderr, "%s", trail[i]->name());
    }
    fprintf(stderr, "\n");
}


template <typename T>
void printElement(T t) {
    cout << left << setw(15) << setfill(' ') << t;
}


void Solver::displayHeaderCurrentSearchSpace() {
    if(verbose.verbosity < 1)
        return;
    printf("c ");
    printElement("conflicts");
    printElement("decisions");
    printElement("filterCalls");
    printElement("propagations");
    printElement("values");
    printElement("Nogoods");

    printElement("min/max/avg depth");
    cout << endl;
}


void Solver::displayCurrentSearchSpace() {
    printf("c ");
    printElement(conflicts);
    printElement(decisions);
    printElement(filterCalls);
    printElement(propagations);
    printElement(problem.nbValues());
    if(nogoodsFromRestarts)
        printElement(noGoodsEngine->statistics[nbnogoods]);
    else
        printElement(0);


    printElement(std::to_string(localstatistics[minDepth]) + "   " + std::to_string(localstatistics[maxDepth]) + "   " +
                 std::to_string(localstatistics[sumDepth] / (localstatistics[nbConflicts] + 1)));
    cout << endl;

    for(int i = 0; i < NBLOCALSTATS; i++) localstatistics[i] = 0;
}


void Solver::interrupt() { }


void Solver::printFinalStats() {
    double cpu_time = cpuTime();

    printf("c restarts              : %lu\n", statistics[restarts]);
    printf("c decisions             : %lu (%.0f /sec)\n", decisions, decisions / cpu_time);
    printf("c wrong decisions       : %lu (%.0f /sec)\n", wrongDecisions, wrongDecisions / cpu_time);
    printf("c conflicts             : %lu (%.0f /sec)\n", conflicts, conflicts / cpu_time);
    printf("c propagations          : %lu (%.0f /sec)\n", propagations, propagations / cpu_time);
    printf("c root propagations     : %lu\n", statistics[rootPropagations]);
    printf("c filter calls          : %lu   (%.0f /sec)\n", filterCalls, filterCalls / cpu_time);
    printf("c useless filter calls  : %lu   (%lu %%)\n", statistics[uselessFilterCalls],
           statistics[uselessFilterCalls] * 100 / filterCalls);
    if(nogoodsFromRestarts)
        noGoodsEngine->printStats();
    if(doProfiling)
        profiling->display();
}
