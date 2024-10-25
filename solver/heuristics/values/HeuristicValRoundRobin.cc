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

HeuristicValRoundRobin::HeuristicValRoundRobin(Solver &s, std::string &sq) : HeuristicVal(s) {
    sequence = sq;
    s.addObserverDeleteDecision(this);

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
    current = 0;
}

int HeuristicValRoundRobin::select(Variable *x) { return heuristics[current]->select(x); }

void HeuristicValRoundRobin::notifyFullBacktrack() {
    current++;
    if(current >= sequence.size())
        current = 0;
    std::string s = std::string(" - Val=") + std::string(1, sequence[current]);
    solver.verbose.log(NORMAL, s.c_str());
}