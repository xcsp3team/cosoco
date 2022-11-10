#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <solver/heuristics/values/ForceIdv.h>

#include "ObjectiveConstraint.h"
#include "Solution.h"
#include "core/OptimizationProblem.h"
#include "core/Problem.h"
#include "solver/AbstractSolver.h"
#include "solver/Solver.h"

namespace Cosoco {


class Optimizer : public AbstractSolver, ObserverConflict {
   protected:
    pFactory::Communicator<long> *boundCommunicator;
    long                          lower, upper;   // The current lower et upper bound

    Solution *bestSolution;   // The solution manager (used to avoid problems if the solver is killed during solution storing
    long      best;           // Best value until now
   public:
    bool                 boundFound;       // One solution has been found?
    bool                 invertBestCost;   // usefull with sum and minimize, we invert the cost due to SumGE implementation only
    int                  callToSolver;     // The nb calls to the SAT solver
    Solver *             solver;           // The solver used to optimize
    ObjectiveConstraint *objectiveLB;      // ctr >= k
    ObjectiveConstraint *objectiveUB;      // ctr <= k
    OptimisationType     optimtype;        // Minimize or Maximize
    bool                 useDicothomicMode;   // TODO: class inheritence ?
    bool                 progressSaving;
    bool                 firstCall;

    Optimizer(Problem &p);

    int  solve(vec<RootPropagation> &assumps) override;
    void printFinalStats() override;
    void displayCurrentSolution() override;

    int solveInOneDirection(vec<RootPropagation> &assumps);

    void setSolver(Solver *s, Solution *solution);


    double realTimeForBestSolution() { return bestSolution->realTimeForBestSolution; }


    long bestCost() { return bestSolution->bestBound(); }


    void setGroup(pFactory::Group *pthreadsGroup, pFactory::Communicator<RootPropagation *> *rpc) override {
        assert(solver != nullptr);
        AbstractSolver::setGroup(pthreadsGroup, rpc);
        solver->setGroup(pthreadsGroup, rpc);
    }


    bool hasSolution() override { return bestSolution->exists(); }


    void addProgressSaving() {
        assert(solver != nullptr);
        progressSaving       = true;
        solver->heuristicVal = new ForceIdvs(*solver, solver->heuristicVal, false);
    }


    void addBoundCommunicator(pFactory::Communicator<long> *boundC) {
        // Concurrent mode : register to notification
        solver->addObserverConflict(this);
        boundCommunicator = boundC;
    }

    void importNewBound();

    void notifyConflict(Constraint *c, int level) override;
};
}   // namespace Cosoco

#endif /* OPTIMIZER_H */
