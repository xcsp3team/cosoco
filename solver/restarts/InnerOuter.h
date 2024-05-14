//
// Created by audemard on 14/05/2024.
//

#ifndef COSOCO_INNEROUTER_H
#define COSOCO_INNEROUTER_H

#include "Restart.h"
namespace Cosoco {
class InnerOuterRestart : public Restart {
    uint64_t inner = 100;
    uint64_t outer = 100;
    uint64_t limit = inner;


   public:
    explicit InnerOuterRestart(Solver *s) : Restart(s) { }


    void initialize() override {
        inner = 100;
        outer = 100;
        limit = solver->conflicts + inner;
    }


    bool isItTimeToRestart() override {
        if(solver->conflicts >= limit) {
            if(inner >= outer) {
                outer *= 1.1;
                inner = 100;
            } else
                inner *= 1.1;
            limit += inner;
            return true;
        }
        return false;
    }
};
}   // namespace Cosoco


#endif   // COSOCO_INNEROUTER_H
