//
// Created by audemard on 20/02/23.
//

#ifndef COSOCO_NOGOODSENGINE_H
#define COSOCO_NOGOODSENGINE_H

#include "ObserverDecision.h"
#include "Solver.h"
#include "Tuple.h"
#define NOGOODSSTATS 7
enum NoGoodStats { nbnogoods, size1, size2, sumsize, maxsize, cfl, props };

namespace Cosoco {

class DecisionMarker;

class NoGoodsEngine : public ObserverNewDecision, ObserverDeleteDecision {
    Solver                        &solver;
    vec<vec<Tuple>>                nogoods;           // list of all nogoods
    std::map<Tuple, int, cmpTuple> watcherPosition;   // the position pos in th watchers vector of this tuple x!=idv
    vec<vec<int>>                  watchers;          // watchers[pos] provides all nogoods watcherd by the tuple ix!=idv
    vec<Tuple>                     nogoodsOfSize1;    // Store nogoods of size 1 before enqueue
                                                      // them in the solver propagation queue
    vec<Tuple> currentBranch;                         // The current branch of the search tree

   public:
    static Constraint *fake;
    vec<long>          statistics;
    explicit NoGoodsEngine(Solver &s);

    bool generateNogoodsFromRestarts();              // Generate different nogoods
    void addNoGood(vec<Tuple> &nogood);              // Add a no good to the database
    void addWatcher(Tuple &tuple, int ngposition);   // Add watcher
    void enqueueNoGoodsOfSize1();                    // Enqueue nogoods of size 1 in solver queue
    bool propagate(Variable *x);                     // Propagate x=idv in database of nogoods
    bool isSupport(Tuple &tuple);                    // is definitively a support for the nogood

    // Callbacks to store and delete the current branch
    void notifyNewDecision(Variable *x, Solver &s) override;
    void notifyDeleteDecision(Variable *x, int v, Solver &s) override;
    void notifyFullBacktrack() override;


    // Minor functions (display/trace, debug...)
    void displayTuples(vec<Tuple> &ng);
    void printStats();
    void checkWatchers();
};
}   // namespace Cosoco

#endif   // COSOCO_NOGOODSENGINE_H
