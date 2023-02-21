//
// Created by audemard on 20/02/23.
//

#include "NoGoodsEngine.h"
using namespace Cosoco;

Constraint *NoGoodsEngine::fake = (Constraint *)0x1;


NoGoodsEngine::NoGoodsEngine(Solver &s) : solver(s) {
    statistics.growTo(NOGOODSSTATS, 0);
    s.addObserverNewDecision(this);
    s.addObserverDeleteDecision(this);
}

std::ostream &operator<<(std::ostream &stream, Tuple const &tuple) {
    stream << tuple.x->_name << (tuple.eq ? "=" : "!=") << tuple.idv << " ";
    return stream;
}
//-----------------------------------------------------------------------
// -- No good generation and recording
//-----------------------------------------------------------------------

bool NoGoodsEngine::generateNogoodsFromRestarts() {
    nogoodsOfSize1.clear();
    vec<Tuple> nogood;
    for(Tuple &currentDecision : currentBranch) {
        nogood.push(currentDecision);
        if(currentDecision.eq == false) {
            if(currentDecision != currentBranch[0])
                solver.noGoodsEngine->addNoGood(nogood);
            nogood.pop();   // Remove the negative one
        }
    }
    return true;
}


void NoGoodsEngine::addNoGood(vec<Tuple> &nogood) {
    statistics[nbnogoods]++;
    if(nogood.size() == 1) {
        nogoodsOfSize1.push(Tuple(nogood[0]));
        statistics[size1]++;
        return;
    }

    nogoods.push();
    for(auto &t : nogood) {
        nogoods.last().push(t);
        nogoods.last().last().eq = false;   // We want to store this nogood... // TODO to be improved
    }
    addWatcher(nogoods.last()[0], nogoods.size() - 1);
    addWatcher(nogoods.last()[1], nogoods.size() - 1);
}

void NoGoodsEngine::addWatcher(Tuple &ng, int ngposition) {
    int wp = -1;
    if(watcherPosition.count(ng) == 0) {
        watchers.push();                                     // Add a new watcher for x!=idv
        watcherPosition.insert({ng, watchers.size() - 1});   // insert the position in the maps
        wp = watchers.size() - 1;
    } else
        wp = watcherPosition[ng];
    assert(wp >= 0);
    watchers[wp].push(ngposition);   // the tuple watch this nogood
}


//-----------------------------------------------------------------------
// -- No good propagation
//-----------------------------------------------------------------------

bool NoGoodsEngine::propagate(Variable *x) {
    // checkWatchers();
    if(x->size() > 1)
        return true;
    Tuple ng(x, x->valueId());
    if(watcherPosition.count(ng) == 0)   // this tuple does not watch any nogood
        return true;

    int position = watcherPosition[ng];
    int i = 0, j = 0;
    for(; i < watchers[position].size();) {
        int         ngposition    = watchers[position][i++];
        vec<Tuple> &nogood        = nogoods[ngposition];
        int         falsePosition = ng == nogood[0] ? 0 : 1;
        assert(ng == nogood[falsePosition]);

        if(isSupport(nogood[1 - falsePosition])) {   // Ths tuple is satisfied, can pass it
            watchers[position][j++] = ngposition;
            continue;
        }
        for(int k = 2; k < nogood.size(); k++)
            if(nogood[k].x->containsIdv(nogood[k].idv) == false || nogood[k].x->size() > 1) {
                // Find a new watcher (size > 2 or the id is not in the domain
                nogood[falsePosition] = nogood[k];   // put the new watch in good position
                nogood[k]             = ng;
                addWatcher(nogood[falsePosition], ngposition);
                goto nextNoGood;
            }

        watchers[position][j++] = ngposition;   // The nogood stay in the watcher, it has to be propagated
        statistics[props]++;
        if(solver.delIdv(nogood[1 - falsePosition].x, nogood[1 - falsePosition].idv) == false) {   // The nogood is false
            statistics[cfl]++;
            while(i < watchers[position].size())   // Copy the remaining watches
                watchers[position][j++] = watchers[position][i++];

            watchers[position].shrink(i - j);
            return false;
        }
    nextNoGood:;
    }
    watchers[position].shrink(i - j);
    return true;
}

bool NoGoodsEngine::isSupport(Tuple &tuple) { return tuple.x->containsIdv(tuple.idv) == false; }

void NoGoodsEngine::enqueueNoGoodsOfSize1() {
    for(Tuple &t : nogoodsOfSize1) solver.delIdv(t.x, t.idv);
}

//-----------------------------------------------------------------------
// -- Observers callbacks
//-----------------------------------------------------------------------

void NoGoodsEngine::notifyNewDecision(Variable *x, Solver &s) { currentBranch.push({x, x->valueId(), true}); }


void NoGoodsEngine::notifyDeleteDecision(Variable *x, int v, Solver &s) {
    Tuple current = {x, x->domain.toIdv(v), true};
    int   pos     = currentBranch.firstOccurrenceOf(current);
    assert(pos >= 0);
    currentBranch.cut(pos + 1);
    assert(currentBranch.last() == current);
    currentBranch.last().eq = false;
    assert(currentBranch.contains(current) == false);
}


void NoGoodsEngine::notifyFullBacktrack() { currentBranch.clear(); }


//-----------------------------------------------------------------------
// -- Minor methods
//-----------------------------------------------------------------------


void NoGoodsEngine::displayTuples(vec<Tuple> &ng) {
    for(Tuple &tuple : ng) std::cout << tuple;
    std::cout << std::endl;
}

void NoGoodsEngine::printStats() {
    printf("\nc nogoods               : %lu\n", statistics[nbnogoods]);
    printf("c ng size1              : %lu\n", statistics[size1]);
    printf("c ng propagations       : %lu\n", statistics[props]);
    printf("c ng conflicts          : %lu\n", statistics[cfl]);
}

void NoGoodsEngine::checkWatchers() {
    // Check taht all nogoods are watched by the first 2 tuples
    for(int i = 0; i < nogoods.size(); i++) {
        assert(watchers[watcherPosition[nogoods[i][0]]].firstOccurrenceOf(i) >= 0);
        assert(watchers[watcherPosition[nogoods[i][1]]].firstOccurrenceOf(i) >= 0);
    }

    // check the inverse
    for(auto const &p : watcherPosition) {
        for(int tmp : watchers[p.second]) {
            assert(nogoods[tmp][0] == p.first || nogoods[tmp][1] == p.first);
        }
    }

    // Cehck that a nogood is watched only once by a literal
    std::map<int, int> appears;
    for(auto &w : watchers) {
        appears.clear();
        for(int tmp : w) {
            assert(appears.count(tmp) == 0);
            appears.insert({tmp, 1});
        }
    }
}