#include "PortofolioSolver.h"

#include <Groups.h>
#include <optimizer/Optimizer.h>
#include <solver/heuristics/values/HeuristicValLast.h>

using namespace Cosoco;

ParallelSolver::ParallelSolver(Problem &p, bool o) : AbstractSolver(p), nbcores(0), optimize(o), group(nullptr) { }

void ParallelSolver::setSolvers(vec<AbstractSolver *> &s) {
    s.copyTo(solvers);
    nbcores = solvers.size();
    group   = new pFactory::Group(nbcores);
    group->concurrent();

    // rootPropagationsCommunicator = new pFactory::Communicator<int>(group);
    // boundCommunicator = new pFactory::Communicator<long>(group);


    for(auto solver : solvers) {
        solver->setGroup(group, rootPropagationsCommunicator);
        Optimizer *o = nullptr;
        // if((o = dynamic_cast<Optimizer *>(solver)) != nullptr)
        //     o->addBoundCommunicator(boundCommunicator);
    }
}

bool ParallelSolver::hasSolution() {
    for(auto solver : solvers)
        if(solver->hasSolution())
            return true;
    return false;
}


void ParallelSolver::printFinalStats() { solvers[0]->printFinalStats(); }


void ParallelSolver::displayCurrentSolution() {
    for(auto solver : solvers) {
        if(solver->hasSolution()) {
            solver->displayCurrentSolution();
            break;
        }
    }
};


PortofolioSolver::PortofolioSolver(Problem &p, bool optimize) : ParallelSolver(p, optimize) { }


int PortofolioSolver::solve(vec<RootPropagation> &assumps) {
    diversifySolvers();


    for(auto solver : solvers) {
        group->add([=]() { return solver->solve(); });
    }

    group->start();         // Start the tasks
    return group->wait();   // Wait that one task is finished
}


void PortofolioSolver::diversifySolvers() {
    /*for(int core = 0; core < nbcores; core++) {
        Solver    *solver = nullptr;
        Optimizer *o      = nullptr;
        if((o = dynamic_cast<Optimizer *>(solvers[core])) != nullptr)
            solver = o->solver;
        else
            solver = dynamic_cast<Solver *>(solvers[core]);

        solver->addLastConflictReasoning(core % 2 + 1);

        if(core > 0)
            solver->addRandomizationFirstDescent();
        if(core % 2 == 0) {   // Stick - no stick
            solver->addStickingValue();
        }

        if(core == 0 || core == 1)   // Fist val, Last val
            solver->heuristicVal = new HeuristicValLast(*solver);

        // Best version with ValLast 4/5
        if(core == 4 || core == 5)
            solver->heuristicVal = new HeuristicValRandom(*solver);

        // Geometric/luby restart
        solver->addRestart(core < nbcores / 2);

        if(o != nullptr) {
            if(core != 0)
                o->addProgressSaving();
        }
    }
     */
}
