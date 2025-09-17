//
// Created by audemard on 20/02/23.
//

#include "NoGoodsEngine.h"

#include "Solver.h"
#include "System.h"
using namespace Cosoco;

Constraint *NoGoodsEngine::fake = reinterpret_cast<Constraint *>(0x1);


NoGoodsEngine::NoGoodsEngine(Solver &s) : solver(s), maxArity(1000), capacity(0) {
    // std::cout << n1 << " " << n2 << " " << sizeof(long) * 8 << std::endl;

    // if(n1 + n2 <= sizeof(long) * 8)
    auto n1 = static_cast<unsigned int>(ceil(log2(s.problem.nbVariables())));
    auto n2 = static_cast<unsigned int>(ceil(log2(s.problem.maximumDomainSize())));
    if(n1 + n2 > 8 * sizeof(Lit) - 1)
        throw std::runtime_error("Domains and variables are too big to enable Nogood engine");

    OFFSET    = static_cast<unsigned int>(pow(2, n2 + 1));   // +1 because 0 excluded ???
    totalTime = 0;
    statistics.growTo(NOGOODSSTATS, 0);
    s.addObserverNewDecision(this);
    s.addObserverDeleteDecision(this);

    // Manage space
    last    = 0;
    nogoods = static_cast<Lit *>(malloc(sizeof(Lit)));
    enlargeNogoodStructure(1024 * 1024);
}

std::ostream &operator<<(std::ostream &stream, Tuple const &tuple) {
    stream << tuple.x->_name << (tuple.eq ? "=" : "!=") << tuple.idv << " ";
    return stream;
}
//-----------------------------------------------------------------------
// -- Block solution with nogood
//-----------------------------------------------------------------------

void NoGoodsEngine::generateNogoodFromSolution() {
    vec<Lit> nogood;
    for(Variable *x : solver.problem.variables)
        if(x->useless == false)
            nogood.push(getNegativeDecisionFor(x, x->domain.toIdv(x->value())));
    addNoGood(nogood);
}

//-----------------------------------------------------------------------
// -- No good generation and recording
//-----------------------------------------------------------------------

bool NoGoodsEngine::generateNogoodsFromRestarts() {
    nogoodsOfSize1.clear();
    vec<Lit> nogood;
    for(auto &currentDecision : currentBranch) {
        nogood.push(currentDecision);
        if(currentDecision < 0) {
            if(nogood.size() < maxArity && currentDecision != currentBranch[0]) {
                addNoGood(nogood);
                if(solver.threadsGroup != nullptr && nogood.size() < solver.problem.nbVariables() / 20) {
                    std::vector<Lit> tmp;
                    for(auto &l : nogood) tmp.push_back(l);
                    solver.nogoodCommunicator->send(tmp);
                }
            }
            nogood.pop();   // Remove the negative one
        }
    }
    return true;
}

void NoGoodsEngine::enlargeNogoodStructure(unsigned int new_capacity) {   // This part is based on Minisat.
    unsigned int prev_capacity = capacity;
    while(new_capacity == 0 || capacity < new_capacity) {
        unsigned int delta = ((capacity >> 1) + (capacity >> 3) + 2) & ~1;
        capacity += delta;
        // std::cout << "c realloc " << prev_capacity << " => " << capacity << std::endl;
        if(capacity <= prev_capacity)
            throw std::runtime_error("c Out of memory\n");
        if(new_capacity == 0)
            break;
    }
    nogoods = (Lit *)xrealloc(nogoods, sizeof(Lit) * capacity);
}

unsigned int NoGoodsEngine::insertNoGood(vec<Lit> &nogood) {
    if(nogood.size() + 1 + last >= capacity)
        enlargeNogoodStructure();

    unsigned tmp = last;
    for(Lit l : nogood) nogoods[last++] = l < 0 ? l : -l;
    nogoods[last++] = 0;
    return tmp;
}

void NoGoodsEngine::addNoGood(vec<Lit> &nogood) {
    statistics[nbnogoods]++;
    if(nogood.size() == 1) {
        nogoodsOfSize1.push(nogood[0]);
        statistics[size1]++;
        return;
    }
    if(nogood.size() == 2)
        statistics[size2]++;
    statistics[sumsize] += nogood.size();

    if(statistics[maxsize] < nogood.size())
        statistics[maxsize] = nogood.size();

    unsigned int ng = insertNoGood(nogood);

    addWatcher(nogoods[ng], ng);
    addWatcher(nogoods[ng + 1], ng);
}

void NoGoodsEngine::addWatcher(Lit ng, unsigned int ngposition) {
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
    // checkWatchers();  // Debug watchers

    if(x->size() > 1)   // Nothing to do
        return true;

    Lit ng = getNegativeDecisionFor(x, x->valueId());
    if(watcherPosition.count(ng) == 0)   // this tuple does not watch any nogood
        return true;
    double currentTime = realTime();
    int    position    = watcherPosition[ng];
    int    i = 0, j = 0;
    for(; i < watchers[position].size();) {
        unsigned int ngposition    = watchers[position][i++];
        Lit         *nogood        = &(nogoods[ngposition]);
        int          falsePosition = ng == nogood[0] ? 0 : 1;
        assert(ng == nogood[falsePosition]);

        Variable *y   = getVariableIn(nogood[1 - falsePosition]);
        int       idv = getIndexIn(nogood[1 - falsePosition]);
        if(isSupport(y, idv)) {   // Ths tuple is satisfied, can pass it
            watchers[position][j++] = ngposition;
            continue;
        }

        for(int k = 2; nogood[k] != 0; k++) {
            Variable *z    = getVariableIn(nogood[k]);
            int       idv2 = getIndexIn(nogood[k]);
            if(z->containsIdv(idv2) == false || z->size() > 1) {
                // Find a new watcher
                nogood[falsePosition] = nogood[k];   // put the new watch in good position
                nogood[k]             = ng;
                addWatcher(nogood[falsePosition], ngposition);   // ADd it to the watcher list
                goto nextNoGood;
            }
        }
        watchers[position][j++] = ngposition;   // The nogood stay in the watcher, it has to be propagated
        statistics[props]++;
        if(solver.delIdv(y, idv) == false) {   // The nogood is false
            statistics[cfl]++;
            while(i < watchers[position].size())   // Copy the remaining watches
                watchers[position][j++] = watchers[position][i++];

            watchers[position].shrink(i - j);
            totalTime += realTime() - currentTime;
            return false;
        }
    nextNoGood:;
    }
    watchers[position].shrink(i - j);
    totalTime += realTime() - currentTime;
    return true;
}

bool NoGoodsEngine::isSupport(Variable *x, int idv) { return x->containsIdv(idv) == false; }

void NoGoodsEngine::enqueueNoGoodsOfSize1() {
    for(Lit lit : nogoodsOfSize1) {
        Variable *x   = getVariableIn(lit);
        int       idv = getIndexIn(lit);
        solver.delIdv(x, idv);
    }
}

//-----------------------------------------------------------------------
// -- Observers callbacks
//-----------------------------------------------------------------------

void NoGoodsEngine::notifyNewDecision(Variable *x, Solver &s) { currentBranch.push(getPositiveDecisionFor(x, x->valueId())); }


void NoGoodsEngine::notifyDeleteDecision(Variable *x, int v, Solver &s) {
    Lit current = getPositiveDecisionFor(x, x->domain.toIdv(v));
    int pos     = currentBranch.firstOccurrenceOf(current);
    assert(pos >= 0);
    currentBranch.cut(pos + 1);
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


inline Lit NoGoodsEngine::getPositiveDecisionFor(Variable *x, int idv) const { return 1 + idv + OFFSET * x->idx; }


inline Lit NoGoodsEngine::getNegativeDecisionFor(Variable *x, int idv) const { return -getPositiveDecisionFor(x, idv); }


//-----------------------------------------------------------------------
// -- Minor methods
//-----------------------------------------------------------------------


void NoGoodsEngine::displayTuples(vec<Tuple> &ng) {
    for(Tuple &tuple : ng) std::cout << tuple;
    std::cout << std::endl;
}

void NoGoodsEngine::printStats() {
    printf("\nc nogoods               : %lu\n", statistics[nbnogoods]);
    printf("c nogoods sizes         : #1: %lu   #2: %lu   max size: %lu   avg size: %lu\n", statistics[size1], statistics[size2],
           statistics[maxsize], statistics[nbnogoods] == 0 ? 0 : statistics[sumsize] / statistics[nbnogoods]);
    printf("c ng propagations       : %lu\n", statistics[props]);
    printf("c ng conflicts          : %lu\n", statistics[cfl]);
    printf("c time in nogoods       : %5.3f s\n", totalTime);
}

void NoGoodsEngine::checkWatchers() {
    // Check taht all nogoods are watched by the first 2 tuples
    /*for(int i = 0; i < nogoods.size(); i++) {
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
    }*/
}