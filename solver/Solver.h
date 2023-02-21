#ifndef SOLVER_H
#define SOLVER_H

#include <solver/observers/ObserverDomainReduction.h>
#include <solver/restarts/Restart.h>

#include "AbstractSolver.h"
#include "core/Problem.h"
#include "heuristics/values/HeuristicVal.h"
#include "heuristics/variables/HeuristicVar.h"
#include "mtl/SparseSetCounter.h"
#include "mtl/SparseSetMultiLevel.h"
#include "mtl/SparseSetOfVariables.h"
#include "mtl/Vec.h"
#include "nogoods/NoGoodsEngine.h"
#include "observers/ObserverConflict.h"
#include "observers/ObserverDecision.h"
#include "restarts/Restart.h"

namespace Cosoco {
#define NBSTATS 3
enum GlobalStats { rootPropagations, uselessFilterCalls, restarts };

#define NBLOCALSTATS 4
enum OneRunStats { maxDepth, minDepth, sumDepth, nbConflicts };

class Restart;
class NoGoodsEngine;

class Solver : public AbstractSolver {
   public:
    Problem &problem;                 // The problem to solve
    int      nbWishedSolutions = 1;   // The number of solutions to find
    vec<int> lastSolution;            // The last solution found
#ifdef COMPARESOLUTIONS
    vec<vec<int> > allSolutions;
#endif
    // -- Main Stats ------------------------------------------------------------------------
    uint64_t decisions = 0, conflicts = 0, propagations = 0, wrongDecisions = 0;
    uint64_t filterCalls = 0;


    // -- Minor Stats -----------------------------------------------------------------------
    vec<uint64_t> statistics;                          // global statistics
    vec<int>      localstatistics;                     // local statistics: cleared at each restart
    int           displayStatsEveryConflicts = 1000;   // Display GlobalStats every conflicts


    // -- Minor fields ----------------------------------------------------------------------
    double seed          = 91648253;   // A seed value
    bool   checkSolution = true;       // Check solution or not
    bool   filterCallIsUsefull;        // Check if a call to filter call is usefull or not
    bool   colors;                     // Display colors in terminal

    // -- Search ----------------------------------------------------------------------------
    vec<Variable *> trail;   // the trail of variables
    // Each level stores the SET variables of variables touched
    vec<int>             trail_lim;             // the limit for each level
    vec<int>             decisionVariablesId;   // The id of the decision variables
    SparseSetOfVariables unassignedVariables;   // The set of unassigned variables;
    SparseSetOfVariables decisionVariables;
    SparseSetMultiLevel  entailedConstraints;
    bool                 stopSearch = false;   // Stop the search usefull, in // with the optimizer
    bool                 warmStart;
    // -- Heuristics ------------------------------------------------------------------------
    HeuristicVar *heuristicVar;                         // The heuristic to choose variables
    HeuristicVal *heuristicVal;                         // The heuristic to choose values
    Restart      *restart                  = nullptr;   // The restart strategy
    int           intension2extensionLimit = 100000;    // Transform intension -> extension : limit of the cartesian product
    // -- Propagations ----------------------------------------------------------------------
    SparseSetOfVariables queue;                       // Propagation queue
    Constraint          *currentFilteredConstraint;   // The constraint that is filtered
    SparseSetCounter     pickQueueHistory;            // The set of picking variables history

    // -- Observers ----------------------------------------------------------------------
    vec<ObserverConflict *>        observersConflict;          // Classes listen for conflict
    vec<ObserverNewDecision *>     observersNewDecision;       // Classes listen for decisions
    vec<ObserverDeleteDecision *>  observersDeleteDecision;    // Classes listen for decisions
    vec<ObserverDomainReduction *> observersDomainReduction;   // Classes listen for domain reduction
    unsigned long                  timestamp = 0;              // Current timestamp

    //  -- Nogoods from Restarts  -----------------------------------------------------------
    NoGoodsEngine *noGoodsEngine;

    bool nogoodsFromRestarts = false;
    void addNoGoodsFromRestarts();

    // --------------------------------------------------------------------------------------
    // Construction and initialisation
    // --------------------------------------------------------------------------------------

    Solver(Problem &p, int nbc = 0);
    void addLastConflictReasoning(int nVars);
    void addRandomizationFirstDescent();
    void addStickingValue();
    void addRestart(bool luby = false);


    // --------------------------------------------------------------------------------------
    // Main methods
    // --------------------------------------------------------------------------------------

    int  solve(vec<RootPropagation> &assumps) override;   // The main function
    int  search(vec<RootPropagation> &assumps);           // The search function
    bool manageSolution();                                // Return true if the search is finished

    // --------------------------------------------------------------------------------------
    // Decision methods
    // --------------------------------------------------------------------------------------
    void      newDecision(Variable *x, int idv);   // Assign a variable
    Variable *decisionVariableAtLevel(int lvl);    // Return the decision variable
    int       decisionLevel() const;               // Return the decision level
    bool      isAssigned(Variable *x) const;       // Return true if x is assigned


    // --------------------------------------------------------------------------------------
    // Backtrack methods
    // --------------------------------------------------------------------------------------

    void doRestart();
    void backtrack();                           // Backtrack to previous level
    void backtrack(int level);                  // Backtrack to a given level
    void fullBacktrack(bool all = false);       // Backtrack to the root, if all=true remove everything (including root AC)
    void handleFailure(Variable *x, int idv);   // manage backtrack until fix point
    void handleFailure(int level);              // Manage backtrack to  this level (at least)
    void reinitializeConstraints();             // Force constraints to be propagated

    // --------------------------------------------------------------------------------------
    // Propagate methods
    // --------------------------------------------------------------------------------------

    void        addToQueue(Variable *x);                      // Add a variable to queue (side effect if used directly))
    Variable   *pickInQueue();                                // Pick a var in the prop queue
    Constraint *propagate(bool startWithSATEngine = false);   // Propagate the Queue
    Constraint *propagateComplete();                          // fill the queue and propagate everything
    bool        isGACGuaranted();                             // Return trus if GAC is ensured

    void entail(Constraint *c) {
        if(entailedConstraints.isLimitRecordedAtLevel(decisionLevel()) == false)
            entailedConstraints.recordLimit(decisionLevel());
        entailedConstraints.add(c->idc);
    }
    bool isEntailed(Constraint *c) { return entailedConstraints.contains(c->idc); }
    // --------------------------------------------------------------------------------------
    // Domain variable modifications (add/remove values)
    // --------------------------------------------------------------------------------------

    bool delVal(Variable *x, int v);     // Use it to delete variable values
    bool delIdv(Variable *x, int idv);   // Use it to delete variable values

    bool assignToVal(Variable *x, int v);     // Use it to assign variable value
    bool assignToIdv(Variable *x, int idv);   // Use it to assign variable value


    // --------------------------------------------------------------------------------------
    // Multiple values removal
    // --------------------------------------------------------------------------------------

    bool delValuesGreaterOrEqualThan(Variable *x, int v);
    bool delValuesLowerOrEqualThan(Variable *x, int v);
    bool delValuesInDomain(Variable *x, Domain &d);
    bool delValuesNotInDomain(Variable *x, Domain &d);
    bool changeDomain(Variable *x, SparseSet &newIdvalues);
    bool delValuesInRange(Variable *x, int min, int max);   // del values from min to max excluded


    // --------------------------------------------------------------------------------------
    // Observers methods
    // --------------------------------------------------------------------------------------

    void addObserverConflict(ObserverConflict *oc);   // oc wants to know when a conflict is reached
    void notifyConflict(Constraint *c);               // Notify classes that a conflict is reached

    void addObserverNewDecision(ObserverNewDecision *od);         // od wants to know when a decision is added or deleted
    void notifyNewDecision(Variable *x);                          // Notify classes that x is a new decision
    void addObserverDeleteDecision(ObserverDeleteDecision *od);   // od wants to know when a decision is added or deleted
    void notifyDeleteDecision(Variable *x, int v);                // Notify classes that x=v is not a decision,
    void notifyFullBactrack();                                    // Notify classes that we remove all facts

    void addObserverDomainReduction(ObserverDomainReduction *odr);
    void notifyDomainReduction(Variable *x, int idv);    //
    void notifyDomainAssignment(Variable *x, int idv);   //

    // --------------------------------------------------------------------------------------
    // minor methods
    // --------------------------------------------------------------------------------------

    virtual void displayCurrentSolution() override;
    virtual void printFinalStats() override;   // The final stats to print

    void displayHeaderCurrentSearchSpace();
    void displayCurrentSearchSpace();

    void updateStatisticsWithNewConflict();
    bool hasASolution() { return lastSolution.size() > 0; }
    void displayTrail();

    void interrupt();   // Trigger a (potentially asynchronous) interruption of the solver.

    void setDecisionVariables(vec<Variable *> &vars);   // Set the decision variables
};


};   // namespace Cosoco

#endif /* SOLVER_H */
