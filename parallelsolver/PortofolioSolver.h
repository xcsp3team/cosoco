#ifndef COSOCO_PORTOFOLIOSOLVER_HH
#define COSOCO_PORTOFOLIOSOLVER_HH

#include <mtl/Vec.h>

#include "Communicators.h"
#include "Groups.h"
#include "optimizer/Optimizer.h"
#include "solver/AbstractSolver.h"

namespace Cosoco {

class ParallelSolver : public AbstractSolver {
   public:
    int                          nbcores;
    bool                         optimize;
    vec<AbstractSolver *>        solvers;
    pFactory::Group             *group;
    pFactory::Communicator<int> *rootPropagationsCommunicator;
    pFactory::Communicator<int> *boundCommunicator;


    ParallelSolver(Problem &p, bool o);


    void setSolvers(vec<AbstractSolver *> &s);


    virtual bool hasSolution() override;


    virtual void printFinalStats() override;   // The final stats to print


    virtual void displayCurrentSolution() override;   // display the current solution
};


class PortofolioSolver : public ParallelSolver {
   public:
    PortofolioSolver(Problem &p, bool optimize);
    virtual int solve(vec<RootPropagation> &assumps) override;


    void diversifySolvers();
};
}   // namespace Cosoco

#endif   // COSOCO_PORTOFOLIOSOLVER_HH