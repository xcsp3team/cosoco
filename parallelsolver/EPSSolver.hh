#ifndef COSOCO_EPSSOLVER_H
#define COSOCO_EPSSOLVER_H


#include "PortofolioSolver.hh"

namespace Cosoco {

class EPSSolver : public ParallelSolver {
    vec<vec<RootPropagation> > cubes;
    vec<Variable *>            sortedVariables;

    struct domdegLT {
        bool operator()(Variable *x, Variable *y) {
            double vx = ((double)x->size()) / ((double)x->constraints.size());
            double vy = ((double)y->size()) / ((double)y->constraints.size());
            return vx < vy;
        }
    };

   public:
    EPSSolver(Problem &p, bool o) : ParallelSolver(p, o) { }

    void        createCubes();
    void        cartesianProduct(vec<RootPropagation> &cube, int idx, int nbVariables);
    virtual int solve(vec<RootPropagation> &assumps) override;
};
}   // namespace Cosoco

#endif   // COSOCO_EPSSOLVER_H
