//
// Created by audemard on 15/11/22.
//

#ifndef COSOCO_HEURISTICVALROUNDROBIN_H
#define COSOCO_HEURISTICVALROUNDROBIN_H

#include "HeuristicVal.h"
#include "HeuristicValRandom.h"


namespace Cosoco {

class HeuristicValRoundRobin : public HeuristicVal {
    vec<HeuristicVal *> heuristics;

   public:
    HeuristicValRoundRobin(Solver &s, std::string sequence);


    int select(Variable *x) override;
};


}   // namespace Cosoco

#endif   // COSOCO_HEURISTICVALROUNDROBIN_H
