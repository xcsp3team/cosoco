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
    std::cout << sequence << "\n";
    s.addObserverDeleteDecision(this);

    nbrestarts = 1;
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
    current = heuristics.size() - 1;
}

int HeuristicValRoundRobin::select(Variable *x) { return heuristics[current]->select(x); }

void HeuristicValRoundRobin::notifyFullBacktrack() {
    if(solver.statistics[restarts] % nbrestarts == 0) {
        current++;
        if(current >= heuristics.size())
            current = 0;
    }
    // std::cout << "Val : " << current << " ";
}
