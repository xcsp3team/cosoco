//
// Created by audemard on 2019-04-08.
//

#ifndef COSOCO_LUBYRESTART_HH
#define COSOCO_LUBYRESTART_HH


#include "Restart.h"


namespace Cosoco {
class LubyRestart : public Restart {
    double luby(double y, int x) {
        // Find the finite subsequence that contains index 'x', and the
        // size of that subsequence:
        int size, seq;
        for(size = 1, seq = 0; size < x + 1; seq++, size = 2 * size + 1);

        while(size - 1 != x) {
            size = (size - 1) >> 1;
            seq--;
            x = x % size;
        }

        return pow(y, seq);
    }


    unsigned int limit;
    int          factor;
    unsigned int curr_restarts;


   public:
    explicit LubyRestart(Solver *s) : Restart(s), factor(32), curr_restarts(0) { limit = luby(2, curr_restarts) * factor; }


    void initialize() override {
        curr_restarts = 0;
        limit         = solver->conflicts + luby(2, curr_restarts) * factor;
    }


    bool isItTimeToRestart() override {
        if(solver->conflicts >= limit) {
            curr_restarts++;
            limit = solver->conflicts + luby(2, curr_restarts) * factor;
            return true;
        }
        return false;
    }
};
}   // namespace Cosoco

#endif   // COSOCO_LUBYRESTART_HH
