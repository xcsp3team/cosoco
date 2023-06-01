//
// Created by audemard on 15/11/22.
//

#include "HeuristicValRoundRobinBS.h"

#include "HeuristicValASGS.h"
#include "HeuristicValFirst.h"
#include "HeuristicValLast.h"
#include "HeuristicValOccs.h"
#include "solver/Solver.h"

using namespace Cosoco;

HeuristicValRoundRobinBS::HeuristicValRoundRobinBS(Solver &s, std::string &sequence) : HeuristicVal(s) {
    for(auto c : sequence) {
        idvs.push();
        idvs.last().growTo(solver.problem.nbVariables(), -1);

        if(c == 'F')
            heuristics.push(new HeuristicValFirst(s));
        if(c == 'L')
            heuristics.push(new HeuristicValLast(s));
        if(c == 'R')
            heuristics.push(new HeuristicValRandom(s));
        if(c == 'O')
            heuristics.push(new HeuristicValOccs(s));
        if(c == 'A')
            heuristics.push(new HeuristicValASGS(s));
    }
}

int HeuristicValRoundRobinBS::select(Variable *x) {
    int           nb = solver.statistics[restarts] % heuristics.size();
    HeuristicVal *h  = heuristics[nb];
    if(x->size() == 1)
        return x->domain[0];


    int lv = idvs[nb][x->idx];
    if(lv != -1 && x->domain.containsIdv(lv))
        return lv;
    return h->select(x);
}


void HeuristicValRoundRobinBS::setIdValues(vec<int> &v) {
    int nb = solver.statistics[restarts] % heuristics.size();
    v.copyTo(idvs[nb]);
}
