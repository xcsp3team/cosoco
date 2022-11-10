#include "./Solver.h"

#include <solver/heuristics/values/HeuristicValLast.h>
#include <solver/heuristics/values/HeuristicValRandom.h>
#include <solver/heuristics/variables/HeuristicVarCACD.h>
#include <solver/heuristics/variables/HeuristicVarFirst.h>
#include <solver/restarts/GeometricRestart.h>
#include <solver/restarts/LubyRestart.h>
#include <utils/System.h>

#include <iomanip>

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


Solver::Solver(Problem &p, int nbc)
    : AbstractSolver(p),
      problem(p),
      nbWishedSolutions(1),
      decisions(0),
      conflicts(0),
      propagations(0),
      wrongDecisions(0),
      filterCalls(0),
      displayStatsEveryConflicts(10000),
      seed(91648253),
      checkSolution(true),
      unassignedVariables(p.nbVariables(), p.variables, true),
      decisionVariables(p.nbVariables(), p.variables, true),
      entailedConstraints(p.nbConstraints(), false),
      stopSearch(false),
      restart(nullptr),
      intension2extensionLimit(100000),
      queue(p.nbVariables(), p.variables),
      timestamp(0),
      nogoodsFromRestarts(false),
      satWrapper(nullptr) {
    heuristicVar = new HeuristicVarDomWdeg(*this);
    // new HeuristicVarDomWdeg(*this); //new LastConflictReasoning(*this, new HeuristicVarDomWdeg(*this));
    heuristicVal = new HeuristicValFirst(*this);
    // new HeuristicValLast(*this);//new HeuristicValFirst(*this);//new HeuristicValRandom(*this);//;new HeuristicValFirst(*this);
    statistics.growTo(NBSTATS, 0);
    localstatistics.growTo(NBLOCALSTATS, 0);
    problem.attachSolver(this);
    lastSolutionRun = 0;
    nbSolutions = 0;
    warmStart = false;
}


void Solver::addLastConflictReasoning(int nVars) { heuristicVar = new LastConflictReasoning(*this, heuristicVar, nVars); }


void Solver::addRandomizationFirstDescent() { heuristicVar = new RandomizeFirstDescent(*this, heuristicVar); }


void Solver::addStickingValue() { heuristicVal = new HeuristicValStickingValue(*this, heuristicVal); }


void Solver::addRestart(bool luby) {
    if(luby)
        restart = new LubyRestart(this);
    else
        restart = new GeometricRestart(this);
}


void Solver::enableSATEngine() {
    satWrapper          = new SatWrapper(this);
    decisionMarker      = new DecisionMarker(this);
    nogoodsFromRestarts = true;
    for(Constraint *c : problem.constraints) c->addInitialSATClauses();
}


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
    if(statistics[rootPropagations] == 0)
        statistics[rootPropagations] = propagations;
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
    int nb = 0;
    for(Variable *x : problem.variables)
        if(x->useless) {
            nb++;
            unassignedVariables.del(x);
            decisionVariables.del(x);
        }
    if(nb > 0)
        printf("c remove %d useless variables\n", nb);

    return search(assumps);
}


int Solver::search(vec<RootPropagation> &assumptions) {
    std::vector<RootPropagation *> sharedPropagations, sharedPropagationsNC;

    while(status == RUNNING) {
        if(threadsGroup != nullptr && threadsGroup->isStopped())
            return R_UNKNOWN;

        if(threadsGroup != nullptr && (decisionLevel() == 0 || conflicts % 1000 == 0)) {
            rootPropagationsCommunicator->recvAll(sharedPropagations);

            // Fact to propagate
            if(decisionLevel() > 0 && sharedPropagations.size() > 0)
                doRestart();

            bool ok = true;
            while(!sharedPropagations.empty()) {
                RootPropagation *rp = sharedPropagations.back();
                sharedPropagations.pop_back();
                if(rp->equal == false)
                    ok = delIdv(problem.variables[rp->idx], rp->idv);
                else
                    ok = assignToIdv(problem.variables[rp->idx], rp->idv);
            }

            if(!ok) {   // We propagate a false fact at decision level 0 !!
                return R_UNSAT;
            }
        }


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


    //if(nbSolutions == 1)
    //    verbose.log(NORMAL, "c First solution is found\n");

    if(checkSolution)
        problem.checkSolution();
    lastSolution.clear();   // Store the last solution

    for(Variable *x : problem.variables) lastSolution.push(x->useless ? 0 : x->value());


    if(nbSolutions == nbWishedSolutions) {
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
}


Variable *Solver::decisionVariableAtLevel(int lvl) {
    assert(lvl > 0);
    assert(decisionLevel() >= lvl);
    Variable *x = (trail[trail_lim[lvl - 1]]);
    assert(x->size() <= 1);   // Domain of decision variable can be empty
    assert(unassignedVariables.contains(x) == false);
    return x;
}


int Solver::decisionLevel() { return trail_lim.size(); }


bool Solver::isAssigned(Variable *x) { return unassignedVariables.contains(x) == false; }

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
        reinitializeConstraints();
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
    }
    if(round >= 1 && satWrapper != nullptr)
        satWrapper->propagateAssertive = false;
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
    if(satWrapper != nullptr)
        satWrapper->startCSPPropagatations();
    if(nogoodsFromRestarts && decisionMarker->generateNogoodsFromRestarts() == false)
        status = FULL_EXPLORATION;
    statistics[restarts]++;
    if(verbose.verbosity >= 1)
        displayCurrentSearchSpace();
    fullBacktrack();
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


Constraint *Solver::propagate(bool startWithSATEngine) {
    currentFilteredConstraint = nullptr;
startAgainPropagationProcess:

    if(startWithSATEngine == false && satWrapper != nullptr)
        satWrapper->startCSPPropagatations();
    if(startWithSATEngine == false) {
        while(queue.size() > 0) {
            Variable *x = pickInQueue();
            if(x->size() == 0)
                return SatWrapper::fake;   // What's happen if satWrapper->addclause -> props -> end of search ???
            for(Constraint *c : x->constraints) {
                if(x->timestamp > c->timestamp && isEntailed(c) == false) {
                    filterCalls++;
                    filterCallIsUsefull       = false;
                    currentFilteredConstraint = c;
                    if(c->filterFrom(x) == false) {   // Inconsistent
                        c->timestamp = ++timestamp;
                        notifyConflict(c);
                        currentFilteredConstraint = nullptr;
                        return c;
                    }
                    currentFilteredConstraint = nullptr;
                    if(filterCallIsUsefull == false)
                        statistics[uselessFilterCalls]++;
                    c->timestamp = ++timestamp;
                }
            }
        }
    }
    startWithSATEngine = false;
    if(satWrapper != nullptr) {
        uint64_t nbProps = propagations;
        int      bt      = satWrapper->doSATPropagations();   // perform SAT Propagations
        while(bt != SATISOK) {
            handleFailure(bt);
            SatWrapper::SatPropState st = satWrapper->addAssertiveLiteralInQueue();
            if(st == SatWrapper::USELESS)
                bt = SATISOK;
            if(st == SatWrapper::CONFLICT)
                bt = decisionLevel() - 1;
            if(st == SatWrapper::PROPAGATE) {
                startWithSATEngine             = true;
                satWrapper->propagateAssertive = false;
                goto startAgainPropagationProcess;
            }
        }
        if(satWrapper->providesNewSATPropagations() == false) {
            statistics[SATConflicts]++;
            return SatWrapper::fake;
        }
        if(queue.size() > 0) {   // Do it!
            statistics[SATPropagations] += (propagations - nbProps);
            goto startAgainPropagationProcess;
        }
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
    if(threadsGroup != nullptr && decisionLevel() == 0)
        rootPropagationsCommunicator->send(new RootPropagation(x->idx, false, idv));

    filterCallIsUsefull = true;

    notifyDomainReduction(x, idv);
    x->delIdv(idv, decisionLevel());

    if(x->addToTrail)
        trail.push(x);
    x->addToTrail = false;
    propagations++;
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
    filterCallIsUsefull = true;
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
    if(threadsGroup != nullptr && decisionLevel() == 0)
        rootPropagationsCommunicator->send(new RootPropagation(x->idx, true, idv));

    // if(d.size()==1) return true;
    filterCallIsUsefull = true;
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
    for(int idv : x->domain) {
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
    if (start >= stop)
        return true;
    int first = x->minimum(), last = x->maximum();
    if (start > last || stop < first)
        return true; // because there is no overlapping
    int left = std::max(start, first), right = std::min(stop - 1, last);
    if (left == first && right == last)
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
    printElement("Root Prop.");
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
    printElement(trail_lim[0]);
    printElement(statistics[nogoods]);


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
    if(satWrapper != nullptr) {
        printf("c\n");
        printf("c clauses               : %d\n", satWrapper->satSolver->nClauses());
        printf("c SAT propagations      : %lu\n", statistics[SATPropagations]);
        printf("c SAT conflicts         : %lu (%lu learnts)\n", statistics[SATConflicts], statistics[SATLearnts]);
    }
}
