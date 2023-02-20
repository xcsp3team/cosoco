//
// Created by audemard on 20/02/23.
//

#include "NoGoodsEngine.h"

using namespace Cosoco;

Constraint *NoGoodsEngine::fake = (Constraint *)0x1;


NoGoodsEngine::NoGoodsEngine(Solver &s) : solver(s) {
    statistics.growTo(NOGOODSSTATS, 0);
    unsigned int n1 = (int)ceil(log2(s.problem.nbVariables()));
    unsigned int n2 = (int)ceil(log2(s.problem.maximumDomainSize()));
    // std::cout << n1 << " " << n2 << " " << sizeof(long) * 8 << std::endl;

    // if(n1 + n2 <= sizeof(long) * 8)
    //     assert(false);   // Number of bits....

    OFFSET = (int)pow(2, n2 + 1);   // +1 because 0 excluded ???
    s.addObserverNewDecision(this);
    s.addObserverDeleteDecision(this);
}

//-----------------------------------------------------------------------
// -- No good generation and recording
//-----------------------------------------------------------------------

bool NoGoodsEngine::generateNogoodsFromRestarts() {
    nogoodsOfSize1.clear();
    vec<Tuple> nogood;
    // printf("ICI\n");
    //  displayCurrentBranch();


    for(int currentDecision : currentBranch) {
        nogood.push(Tuple(getVariableIn(currentDecision), getIndexIn(currentDecision)));
        if(currentDecision < 0) {
            if(currentDecision != currentBranch[0]) {
                // for(auto &t : nogood) std::cout << t.x->_name << "!= " << t.idv;
                // std::cout << "\n";
                solver.noGoodsEngine->addNoGood(nogood);
            }
            nogood.pop();   // Remove the negative one
                            // std::cout << std::endl;
            //            return;
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
    for(auto &t : nogood) nogoods.last().push(t);
    addWatcher(nogoods.last()[0], nogoods.size() - 1);
    addWatcher(nogoods.last()[1], nogoods.size() - 1);
}

void NoGoodsEngine::addWatcher(Tuple &ng, int ngposition) {
    int position = 0;
    if(watcherPosition.count(ng) == 0) {
        watchers.push();                                     // Add a new watcher for x!=idv
        watcherPosition.insert({ng, watchers.size() - 1});   // insert the position in the maps
    } else
        position = watcherPosition[ng];
    watchers[position].push(ngposition);   // the tuple watch this nogood
}


//-----------------------------------------------------------------------
// -- No good propagation
//-----------------------------------------------------------------------

bool NoGoodsEngine::propagate(Variable *x) {
    if(x->size() > 1)
        return true;
    Tuple ng(x, x->valueId());
    if(watcherPosition.count(ng) == 0)   // this tuple does not watch any nogoods
        return true;
    int       position = watcherPosition[ng];
    vec<int> &watched  = watchers[position];
    int       i = 0, j = 0;
    for(; i < watched.size();) {
        i++;
        vec<Tuple> &nogood = nogoods[i];
        if(ng != nogood[1]) {   // The  tuple to propagate is in position 0
            assert(ng == nogood[0]);
            nogood[0] = nogood[1];
            nogood[1] = ng;
        }

        if(isSupport(nogood[0])) {   // Ths tuple is satisfied, can pass it
            watched[j++] = watched[i - 1];
            continue;
        }
        for(int k = 2; k < nogood.size(); k++)
            if(nogood[k].x->containsIdv(nogood[k].idv) == false || nogood[k].x->size() > 1) {
                // Find a new watcher (size > 2 or the id is not in the domain
                nogood[1] = nogood[k];   // put the new watch in good position
                nogood[k] = ng;
                addWatcher(nogood[k], position);
                goto nextNoGood;
            }

        watched[j++] = watched[i - 1];   // The nogood stay in the watcher, it has to be propagated
        statistics[props]++;
        if(solver.delIdv(nogood[0].x, nogood[0].idv) == false) {   // The nogood is false
            statistics[cfl]++;
            while(i < watched.size())   // Copy the remaining watches
                watched[j++] = watched[i++];
            return false;
        }
    nextNoGood:;
    }
    watched.shrink(i - j);
    return true;
}

bool NoGoodsEngine::isSupport(Tuple &tuple) { return tuple.x->containsIdv(tuple.idv) == false; }

void NoGoodsEngine::enqueueNoGoodsOfSize1() {
    for(Tuple &t : nogoodsOfSize1) solver.delIdv(t.x, t.idv);
}

//-----------------------------------------------------------------------
// -- Observers callbacks
//-----------------------------------------------------------------------

void NoGoodsEngine::notifyNewDecision(Variable *x, Solver &s) { currentBranch.push(getPositiveDecisionFor(x, x->valueId())); }


void NoGoodsEngine::notifyDeleteDecision(Variable *x, int v, Solver &s) {
    int current = getPositiveDecisionFor(x, x->domain.toIdv(v));
    assert(currentBranch.firstOccurrenceOf(current) >= 0);
    currentBranch.cut(currentBranch.firstOccurrenceOf(current) + 1);
    assert(currentBranch.last() == current);
    currentBranch.last() = -currentBranch.last();
    assert(currentBranch.contains(current) == false);
}


void NoGoodsEngine::notifyFullBacktrack() { currentBranch.clear(); }


//-----------------------------------------------------------------------
// -- From x=idv to int and vice-versa
//-----------------------------------------------------------------------

Variable *NoGoodsEngine::getVariableIn(int number) { return solver.problem.variables[abs(number) / OFFSET]; }

inline int NoGoodsEngine::getIndexIn(int number) const { return abs(number) % OFFSET - 1; }


inline int NoGoodsEngine::getPositiveDecisionFor(Variable *x, int idv) const { return 1 + idv + OFFSET * x->idx; }


inline int NoGoodsEngine::getNegativeDecisionFor(Variable *x, int idv) const { return -getPositiveDecisionFor(x, idv); }

//-----------------------------------------------------------------------
// -- Minor methods
//-----------------------------------------------------------------------

std::string NoGoodsEngine::getStringFor(long dec) {
    return getVariableIn(dec)->_name + (dec > 0 ? "=" : "!=") + std::to_string(getIndexIn(dec));
}

void NoGoodsEngine::displayCurrentBranch() {
    for(int i = 0; i < currentBranch.size(); i++) {
        std::cout << getStringFor(currentBranch[i]) << " ";
    }
    std::cout << std::endl;
}

void NoGoodsEngine::displayNogood() { }

void NoGoodsEngine::printStats() {
    printf("\nc nogoods               : %lu\n", statistics[nbnogoods]);
    printf("c ng size1              : %lu\n", statistics[size1]);
    printf("c ng propagations       : %lu\n", statistics[props]);
    printf("c ng conflicts          : %lu\n", statistics[cfl]);
}