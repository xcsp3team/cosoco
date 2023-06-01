//
// Created by audemard on 15/11/22.
//

#ifndef COSOCO_HEURISTICVALROUNDROBINBS_H
#define COSOCO_HEURISTICVALROUNDROBINBS_H

#include "HeuristicVal.h"
#include "HeuristicValRandom.h"


namespace Cosoco {

class HeuristicValRoundRobinBS : public HeuristicVal {
    vec<HeuristicVal *> heuristics;
    vec<vec<int> >      idvs;

   public:
    HeuristicValRoundRobinBS(Solver &s, std::string &sequence);

    void setIdValues(vec<int> &v);

    int select(Variable *x) override;
};


}   // namespace Cosoco

#endif   // COSOCO_HEURISTICVALROUNDROBIN_H
