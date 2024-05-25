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
    std::string sequence = "WPFC";
    current              = 0;
    nbrestarts           = 10;
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
}


Variable *HeuristicVarRoundRobin::select() { return heuristics[current]->select(); }


void HeuristicVarRoundRobin::notifyFullBacktrack() { std::cout << solver.statistics[restarts] << std::endl; }
