//
// Created by audemard on 2019-04-29.
//

#include "EPSSolver.hh"

#include "core/Problem.h"
#include "mtl/Sort.h"
#include "solver/AbstractSolver.h"
using namespace Cosoco;

#define NBCUBES 5000


void EPSSolver::cartesianProduct(Cosoco::vec<Cosoco::RootPropagation> &cube, int idx, int nbVariables) {
    if(idx == nbVariables) {
        assert(cube.size() == nbVariables);
        cubes.push();
        for(int i = 0; i < cube.size(); i++) cubes.last().push(RootPropagation(cube[i].idx, true, cube[i].idv));
        return;
    }
    for(int idv = 0; idv < sortedVariables[idx]->size(); idv++) {
        cube.push(RootPropagation(sortedVariables[idx]->idx, true, idv));
        cartesianProduct(cube, idx + 1, nbVariables);
        cube.pop();
    }
}


void EPSSolver::createCubes() {
    int nbV      = 0;
    int nbTuples = 1;

    problem.variables.copyTo(sortedVariables);
    sort(sortedVariables, domdegLT());

    for(Variable *x : sortedVariables) {
        double vx = ((double)x->size()) / ((double)x->constraints.size());
        printf("%g\n", vx);
    }

    do {
        nbTuples *= sortedVariables[nbV++]->domain.size();
    } while(nbTuples < NBCUBES && nbV < problem.variables.size() - 1);

    vec<RootPropagation> cube;
    cartesianProduct(cube, 0, nbV);

    printf("c NB cubes to solve: %d\n", cubes.size());
}


int EPSSolver::solve(vec<RootPropagation> &assumps) {
    // Subproblems creation
    createCubes();
    // All solvers are available
    pFactory::SafeQueue<AbstractSolver *> *availableSolvers = new pFactory::SafeQueue<AbstractSolver *>();
    for(auto solver : solvers) availableSolvers->push_back(solver);


    // Tasks creation
    for(int i = 0; i < cubes.size(); i++) {
        group->add([=]() {
            AbstractSolver *solver = availableSolvers->pop_back();
            int             result = solver->solve(cubes[i]);
            availableSolvers->push_back(solver);

            if(!optimize && result == R_SAT)
                group->stop();
            return result;
        });
    }
    // Start of tasks
    group->start();


    group->wait();   // Wait that one task is finished

    std::vector<int> &returnCodes = group->getReturnCodes();
    int               nbUnsat     = 0;
    int               nbOpt       = 0;
    int               nbSAT       = 0;
    for(int code : returnCodes) {
        if(code == R_UNSAT)
            nbUnsat++;
        if(code == R_OPT)
            nbOpt++;
        if(code == R_SAT)
            nbSAT++;
    }

    if(!optimize && nbSAT > 0)
        return R_SAT;
    if(nbUnsat == cubes.size())
        return R_UNSAT;
    printf("unsat=%d sat=%d opt=%d cubes=%d\n", nbUnsat, nbSAT, nbOpt, cubes.size());
    if(nbUnsat == cubes.size())
        return R_UNSAT;
    if(optimize && nbOpt > 0)
        return R_OPT;

    return R_UNKNOWN;
}