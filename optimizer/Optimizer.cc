#include "Optimizer.h"

#include "core/OptimizationProblem.h"
#include "solver/restarts/Restart.h"
#include "solver/utils/Options.h"

using namespace Cosoco;


Optimizer::Optimizer(Problem &p)
    : AbstractSolver(p),
      invertBestCost(false),
      callToSolver(0),
      solver(nullptr),
      objectiveLB(nullptr),
      objectiveUB(nullptr),
      useDicothomicMode(false),
      progressSaving(false) { }


void Optimizer::setSolver(Solver *s, Solution *solution) {
    solver       = s;
    bestSolution = solution;
    solver->setVerbosity(1);
    optimtype   = (static_cast<OptimizationProblem &>(solver->problem)).type;
    objectiveLB = (static_cast<OptimizationProblem &>(solver->problem)).objectiveLB;
    objectiveUB = (static_cast<OptimizationProblem &>(solver->problem)).objectiveUB;
    assert(objectiveLB != nullptr || objectiveUB != nullptr);
    lower                    = (objectiveLB != nullptr) ? objectiveLB->minLowerBound() : objectiveUB->minLowerBound();
    upper                    = (objectiveLB != nullptr) ? objectiveLB->maxUpperBound() : objectiveUB->maxUpperBound();
    best                     = optimtype == Minimize ? LONG_MAX : LONG_MIN;
    solution->invertBestCost = invertBestCost;
    solution->optimType      = optimtype;
    if(options::boolOptions["bs"].value)
        addProgressSaving();
}


int Optimizer::solve(vec<RootPropagation> &assumps) {
    if(solver == nullptr)
        throw std::runtime_error("Solver is not attached to the optimizer!");
    return solveInOneDirection(assumps);
}


int Optimizer::solveInOneDirection(vec<RootPropagation> &assumps) {
    // Bounds are correctly initialized
    firstCall = true;
    vec<int>             tuple;
    ObjectiveConstraint *objective = (optimtype == Minimize) ? objectiveUB : objectiveLB;
    auto                 c         = dynamic_cast<Constraint *>(objective);
    assert(objective != nullptr);
    status                = RUNNING;
    c->isDisabled         = true;   // Disable the objective
    solver->checkSolution = true;
    while(status == RUNNING) {
        if(optimtype == Minimize)
            objective->updateBound(upper);
        else
            objective->updateBound(lower);

        solver->stopSearch = false;


        int ret = solver->solve(assumps);

        c->isDisabled = false;   // Enable the objective

        callToSolver++;
        if(solver->hasASolution() || (ret == R_UNKNOWN && solver->stopSearch)) {
            nbSolutions++;
            firstCall = false;

            if(solver->restart != nullptr)
                solver->restart->initialize();

            if(solver->hasSolution()) {
                c->extractConstraintTupleFromInterpretation(solver->lastSolution, tuple);
                best = objective->computeScore(tuple);

                // Store solution in order to avoid a signal
                bestSolution->begin(best);
                for(int i = 0; i < solver->lastSolution.size(); i++)
                    bestSolution->appendTo(i, solver->problem.variables[i]->useless ? STAR : solver->lastSolution[i]);
                bestSolution->end(options::boolOptions["colors"].value);

                if(progressSaving) {
                    vec<int> idvalues;
                    idvalues.growTo(solver->problem.nbVariables());
                    for(auto &x : solver->problem.variables) {
                        idvalues[x->idx] = solver->problem.variables[x->idx]->domain[0];
                    }
                    ((ForceIdvs *)solver->heuristicVal.get())->setIdValues(idvalues);
                }
            }

            if(optimtype == Minimize)
                upper = best - 1;
            else
                lower = best + 1;

            solver->fullBacktrack();
            solver->reinitializeConstraints();

        } else {
            status = OPTIMUM;
            if(firstCall)
                return R_UNSAT;
        }
        firstCall = false;
    }

    if(nbSolutions > 0)
        return R_OPT;


    return R_SAT;
}


void Optimizer::notifyConflict(Constraint *c, int level) { }


void Optimizer::displayCurrentSolution() {
    if(bestSolution->exists())
        bestSolution->display();
}


void Optimizer::printFinalStats() {
    printf("c best bound            : %ld\n\n", bestCost());
    solver->printFinalStats();
}
