#ifndef ABSTRACTSOLVER_H
#define ABSTRACTSOLVER_H

#include "Communicators.h"
#include "Groups.h"
#include "core/Problem.h"
#include "utils/Verbose.h"

namespace Cosoco {

#define R_UNKNOWN -1
#define R_SAT     1
#define R_UNSAT   2
#define R_OPT     3

enum STATE { RUNNING, REACH_GOAL, FULL_EXPLORATION, OPTIMUM };
typedef long long Lit;


class RootPropagation {   // x equal idv ??
   public:
    int  idx;   // The index of the variable (the same for all threads)
    bool equal;
    int  idv;
    RootPropagation(int x, bool e, int v) : idx(x), equal(e), idv(v) { }
};

class AbstractSolver {
   public:
    Problem                                  &problem;       // The problem to solve
    int                                       core;          // The id of the core (used in // track)
    STATE                                     status;        // The status of the solver
    int                                       nbSolutions;   // Number of solutions already found
    int                                       lastSolutionRun;
    Verbose                                   verbose;       // The level of verbose mode 0..3
    double                                    random_seed;   // The seed used by the solver
    pFactory::Group                          *threadsGroup;
    pFactory::Communicator<RootPropagation>  *rootPropagationsCommunicator;
    pFactory::Communicator<std::vector<Lit>> *nogoodCommunicator;
    bool                                      firstPropagations;
    inline int                                solve() {
        vec<RootPropagation> assumps;
        return solve(assumps);
    }
    virtual int  solve(vec<RootPropagation> &assumps) = 0;
    virtual void printFinalStats()                    = 0;   // The final stats to print
    virtual void printIntermediateStats() { }                // The intermediate stats to print

    virtual void displayCurrentSolution() = 0;   // displayCurrentBranch the current solution

    AbstractSolver(Problem &pp)
        : problem(pp), core(0), status(RUNNING), nbSolutions(0), random_seed(91648253), threadsGroup(nullptr) { }


    void setVerbosity(int vv) {
        Verbose v;
        verbose.verbosity = vv;
    }
    virtual bool hasSolution() { return nbSolutions > 0; }

    virtual void setGroup(pFactory::Group *pthreadsGroup, pFactory::Communicator<RootPropagation> *rpc,
                          pFactory::Communicator<std::vector<Lit>> *ngc) {
        threadsGroup                 = pthreadsGroup;
        rootPropagationsCommunicator = rpc;
        nogoodCommunicator           = ngc;
    }
};
}   // namespace Cosoco


#endif /* ABSTRACTSOLVER_H */
