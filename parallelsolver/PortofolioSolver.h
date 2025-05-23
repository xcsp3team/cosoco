#ifndef COSOCO_PORTOFOLIOSOLVER_HH
#define COSOCO_PORTOFOLIOSOLVER_HH

#include <mtl/Vec.h>
#include <pFactory/Communicators.h>
#include <pFactory/Parallel.h>

#include "optimizer/Optimizer.h"
#include "solver/AbstractSolver.h"

namespace Cosoco {

class ParallelSolver : public AbstractSolver {
   public:
    int                   nbcores;
    bool                  optimize;
    vec<AbstractSolver *> solvers;
    pFactory::Group      *group;


    ParallelSolver(Problem &p, bool o) : AbstractSolver(p), nbcores(0), optimize(o), group(nullptr) { }


    void setSolvers(vec<AbstractSolver *> &s) {
        s.copyTo(solvers);
        nbcores = solvers.size();
        group   = new pFactory::Group(nbcores);
        pFactory::Communicator<RootPropagation *> *rootPropagationsCommunicator =
            new pFactory::MultipleQueuesCommunicator<RootPropagation *>(group, nullptr);
        pFactory::Communicator<long> *boundCommunicator = new pFactory::MultipleQueuesCommunicator<long>(group, LONG_MAX);


        for(auto solver : solvers) {
            solver->setGroup(group, rootPropagationsCommunicator);
            Optimizer *o = nullptr;
            if((o = dynamic_cast<Optimizer *>(solver)) != nullptr)
                o->addBoundCommunicator(boundCommunicator);
        }
    }


    virtual bool hasSolution() override;


    virtual void printFinalStats() override;   // The final stats to print


    virtual void displayCurrentSolution() override;   // display the current solution
};


class PortofolioSolver : public ParallelSolver {
   public:
    PortofolioSolver(Problem &p, bool optimize) : ParallelSolver(p, optimize) { }
    virtual int solve(vec<RootPropagation> &assumps) override;


    void diversifySolvers();
};
}   // namespace Cosoco

#endif   // COSOCO_PORTOFOLIOSOLVER_HH