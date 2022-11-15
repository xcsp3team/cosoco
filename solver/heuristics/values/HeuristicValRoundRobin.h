//
// Created by audemard on 15/11/22.
//

#ifndef COSOCO_HEURISTICVALROUNDROBIN_H
#define COSOCO_HEURISTICVALROUNDROBIN_H

#include "HeuristicVal.h"
#include "HeuristicValRandom.h"


namespace Cosoco {

class HeuristicValRoundRobin : public HeuristicVal {
    HeuristicValRandom rnd;

   public:
    HeuristicValRoundRobin(Solver &s);


    int select(Variable *x) override;
};


}   // namespace Cosoco

#endif   // COSOCO_HEURISTICVALROUNDROBIN_H
