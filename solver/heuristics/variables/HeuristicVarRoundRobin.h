//
// Created by audemard on 25/05/24.
//

#ifndef COSOCO_HEURISTICVARROUNDROBIN_H
#define COSOCO_HEURISTICVARROUNDROBIN_H

#include "solver/heuristics/variables/HeuristicVar.h"
#include "solver/observers/ObserverDecision.h"
namespace Cosoco {

class HeuristicVarRoundRobin : public HeuristicVar, ObserverDeleteDecision {
    vec<HeuristicVar *> heuristics;
    int                 current;
    int                 nbrestarts;


   public:
    explicit HeuristicVarRoundRobin(Solver &s);
    Variable *select() override;
    void      notifyFullBacktrack() override;
};
}   // namespace Cosoco

#endif   // COSOCO_HEURISTICVARROUNDROBIN_H
