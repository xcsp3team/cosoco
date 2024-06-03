//
// Created by audemard on 25/05/24.
//

#include "HeuristicVarRoundRobin.h"

#include "HeuristicVarCACD.h"
#include "HeuristicVarDomWdeg.h"
#include "HeuristicVarFRBA.h"
#include "PickOnDom.h"

using namespace Cosoco;

HeuristicVarRoundRobin::HeuristicVarRoundRobin(Cosoco::Solver &s) : HeuristicVar(s) {
    s.addObserverDeleteDecision(this);

    std::string sequence = "WPFC";
    nbrestarts           = 10;
    last                 = 0;
    nb                   = 0;
    for(auto c : sequence) {
        if(c == 'W')
            heuristics.push(new HeuristicVarDomWdeg(s));
        if(c == 'P')
            heuristics.push(new PickOnDom(s));
        if(c == 'F')
            heuristics.push(new HeuristicVarFRBA(s));
        if(c == 'C')
            heuristics.push(new HeuristicVarCACD(s));
        for(int i = 1; i < heuristics.size(); i++) heuristics[i]->stop();
    }
    current = 0;   // heuristics.size() - 1;
}


Variable *HeuristicVarRoundRobin::select() { return heuristics[current]->select(); }

void HeuristicVarRoundRobin::notifyNewSolution() {
    last = solver.lastSolutionRun;
    nb   = 0;
}

void HeuristicVarRoundRobin::notifyFullBacktrack() {
    std::cout << current << " " << solver.lastSolutionRun << " " << nb << " " << solver.statistics[restarts] << std::endl;
    nb++;
    if(nb == nbrestarts) {
        // if(solver.statistics[restarts] % nbrestarts == 0) {
        std::cout << "change\n";
        nb = 0;
        heuristics[current]->stop();
        current++;
        if(current >= heuristics.size())
            current = 0;
        heuristics[current]->start();
    }
}
