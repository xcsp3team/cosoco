//
// Created by audemard on 16/02/23.
//

#include "PoolOfHeuristicsValues.h"

#include "Optimizer.h"
#include "Solver.h"
#include "Sum.h"
using namespace Cosoco;

PoolOfHeuristicsValues::PoolOfHeuristicsValues(Solver &s) : HeuristicVal(s) {
    asgs   = new HeuristicValASGS(s);
    first  = new HeuristicValFirst(s);
    last   = new HeuristicValLast(s);
    occs   = new HeuristicValOccs(s);
    random = new HeuristicValRandom(s);
    heuristicForVariable.growTo(s.problem.variables.size(), first);
    selectHeuristics();
}

int PoolOfHeuristicsValues::select(Variable *x) { return heuristicForVariable[x->idx]->select(x); }

void PoolOfHeuristicsValues::selectHeuristics() {
    OptimizationProblem *op = dynamic_cast<OptimizationProblem *>(&(solver.problem));
    if(op != nullptr) {
        ObjectiveConstraint *objective = (op->type == Minimize) ? op->objectiveUB : op->objectiveLB;
        auto                 c         = dynamic_cast<Constraint *>(objective);
        if(c->type == "Sum" && op->type == Maximize) {
            Sum *sum = dynamic_cast<Sum *>(c);
            for(int i = 0; i < c->scope.size(); i++)
                if(sum->coefficients[i] > 0)
                    heuristicForVariable[c->scope[i]->idx] = last;
        }
        if(c->type == "Sum" && op->type == Minimize) {
            Sum *sum = dynamic_cast<Sum *>(c);
            for(int i = 0; i < c->scope.size(); i++)
                if(sum->coefficients[i] < 0)
                    heuristicForVariable[c->scope[i]->idx] = last;
        }
    }

    int nblast = 0, nbfirst = 0, nboccs = 0, nbrandom = 0, nbasgs = 0;
    for(int i = 0; i < heuristicForVariable.size(); i++) {
        if(heuristicForVariable[i] == first)
            nbfirst++;
        if(heuristicForVariable[i] == last)
            nblast++;
        if(heuristicForVariable[i] == occs)
            nboccs++;
        if(heuristicForVariable[i] == random)
            nbrandom++;
        if(heuristicForVariable[i] == asgs)
            nbasgs++;
    }
    if(nbfirst != 0)
        solver.verbose.log(NORMAL, "c nb first val heuristic: %d\n", nbfirst);
    if(nblast != 0)
        solver.verbose.log(NORMAL, "c nb last val heuristic: %d\n", nblast);
    if(nboccs != 0)
        solver.verbose.log(NORMAL, "c nb first val heuristic: %d\n", nboccs);
    if(nbrandom != 0)
        solver.verbose.log(NORMAL, "c nb first val heuristic: %d\n", nbrandom);
    if(nbasgs != 0)
        solver.verbose.log(NORMAL, "c nb first val heuristic: %d\n", nbasgs);
}
