//
// Created by audemard on 15/11/22.
//

#ifndef COSOCO_HEURISTICVALROUNDROBIN_H
#define COSOCO_HEURISTICVALROUNDROBIN_H

#include "HeuristicVal.h"
#include "HeuristicValRandom.h"
#include "ObserverDecision.h"


namespace Cosoco {

class HeuristicValRoundRobin : public HeuristicVal, ObserverDeleteDecision {
   public:
    vec<HeuristicVal *> heuristics;
    HeuristicValRoundRobin(Solver &s, std::string &sequence);
    int current;
    int nbrestarts;

    int  select(Variable *x) override;
    void notifyFullBacktrack() override;
};


}   // namespace Cosoco

#endif   // COSOCO_HEURISTICVALROUNDROBIN_H
