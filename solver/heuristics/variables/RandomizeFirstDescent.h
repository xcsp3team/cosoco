//
// Created by audemard on 2019-04-08.
//

#ifndef COSOCO_RANDOMIZEFIRSTDESCENT_H
#define COSOCO_RANDOMIZEFIRSTDESCENT_H


#include "HeuristicVar.h"
#include "solver/Solver.h"
#include "solver/observers/ObserverConflict.h"


namespace Cosoco {

class RandomizeFirstDescent : public HeuristicVar, ObserverConflict {
    bool          firstDescent;
    HeuristicVar *hvar;   // The main heuristic
   public:
    RandomizeFirstDescent(Solver &s, HeuristicVar *hv);

    // Returns a random float 0 <= x < 1. Seed must never be 0.
    static double drand(double &seed) {
        seed *= 1389796;
        int q = static_cast<int>(seed / 2147483647);
        seed -= static_cast<double>(q) * 2147483647;
        return seed / 2147483647;
    }

    // Returns a random integer 0 <= x < size. Seed must never be 0.
    static int irand(double &seed, int size) { return static_cast<int>(drand(seed) * size); }


    Variable *select() override;
    void      notifyConflict(Constraint *c, int level) override;
};
}   // namespace Cosoco

#endif   // COSOCO_RANDOMIZEFIRSTDESCENT_H
