#include "PortofolioSolver.h"

#include <Groups.h>
#include <optimizer/Optimizer.h>
#include <solver/heuristics/values/HeuristicValLast.h>

#include "HeuristicValFirst.h"
#include "HeuristicValRandom.h"
#include "HeuristicValRoundRobin.h"
#include "HeuristicVarCACD.h"
#include "HeuristicVarDomWdeg.h"
#include "HeuristicVarFRBA.h"
#include "HeuristicVarRoundRobin.h"
#include "Options.h"
#include "PickOnDom.h"

using namespace Cosoco;

ParallelSolver::ParallelSolver(Problem &p, bool o) : AbstractSolver(p), nbcores(0), optimize(o), group(nullptr) { }

void ParallelSolver::setSolvers(vec<AbstractSolver *> &s) {
    s.copyTo(solvers);
    nbcores = solvers.size();
    group   = new pFactory::Group(nbcores);
    group->concurrent();

    rootPropagationsCommunicator = new pFactory::Communicator<RootPropagation>(*group);
    nogoodsCommunicator          = new pFactory::Communicator<std::vector<Lit> >(*group);

    for(auto solver : solvers) {
        solver->setGroup(group, rootPropagationsCommunicator, nogoodsCommunicator);
        Optimizer *o = nullptr;
        if((o = dynamic_cast<Optimizer *>(solver)) != nullptr) {
            o->solver->setGroup(group, rootPropagationsCommunicator, nogoodsCommunicator);
            o->solver->addObserverConflict(o);
            o->solver->checkSolution = false;   // Avoid a concurrent problem. TODO: to be fixed
        }
    }
}

bool ParallelSolver::hasSolution() {
    for(auto solver : solvers)
        if(solver->hasSolution())
            return true;
    return false;
}


void ParallelSolver::printFinalStats() { solvers[0]->printFinalStats(); }


void ParallelSolver::displayCurrentSolution(int verbosity) {
    for(auto solver : solvers) {
        if(solver->hasSolution()) {
            solver->displayCurrentSolution(verbosity);
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
    for(int core = 0; core < nbcores; core++) {
        Solver    *solver = nullptr;
        Optimizer *o      = nullptr;
        if((o = dynamic_cast<Optimizer *>(solvers[core])) != nullptr)
            solver = o->solver;
        else
            solver = dynamic_cast<Solver *>(solvers[core]);
        solver->seed = solver->seed + core * 3;
        if(core == 0)
            solver->verbose = options::intOptions["verb"].value;
        else
            solver->verbose = 0;

        // Core 0 is the default one. Nothing to do.
        if(core > 0) {
            solver->addLastConflictReasoning(core % 2 + 1);
            solver->addRandomizationFirstDescent();
            if(core == 2)
                solver->heuristicVal = new HeuristicValLast(*solver);


            if(core % 5 == 2)
                solver->heuristicVar = new HeuristicVarDomWdeg(*solver);
            if(core % 5 == 3)
                solver->heuristicVar = new PickOnDom(*solver);
            if(core % 5 == 4)
                solver->heuristicVar = new HeuristicVarCACD(*solver);

            if(core == 4)
                solver->heuristicVal = new HeuristicValRandom(*solver);
            if(core == 5)
                solver->heuristicVal = new HeuristicValRoundRobin(*solver, "FLR");
            ;
        }
        if(o == nullptr && core % 2 == 0)   // Stick - no stick
            solver->addStickingValue();
        if(o != nullptr)
            o->addProgressSaving();
    }
}