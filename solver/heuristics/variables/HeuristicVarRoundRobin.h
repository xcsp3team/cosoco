//
// Created by audemard on 25/05/24.
//

#ifndef COSOCO_HEURISTICVARROUNDROBIN_H
#define COSOCO_HEURISTICVARROUNDROBIN_H

#include "HeuristicVar.h"
#include "ObserverDecision.h"
namespace Cosoco {

class HeuristicVarRoundRobin : public HeuristicVar, ObserverDeleteDecision {
    int current;
    int nbrestarts;


   public:
    vec<HeuristicVar *> heuristics;
    explicit HeuristicVarRoundRobin(Solver &s);
    Variable *select() override;
    void      notifyFullBacktrack() override;
};
}   // namespace Cosoco

#endif   // COSOCO_HEURISTICVARROUNDROBIN_H
