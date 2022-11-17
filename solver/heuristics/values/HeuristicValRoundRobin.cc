//
// Created by audemard on 15/11/22.
//

#include "HeuristicValRoundRobin.h"

#include "HeuristicValASGS.h"
#include "HeuristicValFirst.h"
#include "HeuristicValLast.h"
#include "HeuristicValOccs.h"
#include "solver/Solver.h"

using namespace Cosoco;

HeuristicValRoundRobin::HeuristicValRoundRobin(Solver &s, std::string &sequence) : HeuristicVal(s) {
    for(auto c : sequence) {
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

int HeuristicValRoundRobin::select(Variable *x) { return heuristics[solver.statistics[restarts] % heuristics.size()]->select(x); }
