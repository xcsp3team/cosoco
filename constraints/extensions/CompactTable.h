//
// Created by audemard on 30/04/24.
//

#ifndef COSOCO_COMPACTTABLE_H
#define COSOCO_COMPACTTABLE_H


#include "Extension.h"
#include "ObserverDecision.h"
#include "SparseSetMultiLevel.h"
#define BITSET  unsigned long long
#define SIZEW   (8 * sizeof(BITSET))
#define ONLYONE 0xFFFFFFFFFFFFFFFF
#define BONE    1uLL
#define BZERO   0uLL

#define ONETO(n)         ((BONE << n) - 1)
#define ZEROTO(n)        (~((BONE << n) - 1))
#define SETBIT(position) (BONE << position)


namespace Cosoco {

struct pair {
    int level;
    int nb;
};

class CompactTable : public Extension, ObserverDeleteDecision {
   protected:
    vec<int>  SVal;
    vec<int>  SSup;
    vec<int>  lastSizes;
    int       nWords;           // The number of words
    BITSET   *current;          // Current table
    BITSET ***masks;            // masks[x][a] gives the mask for (x,a), used when filtering
    BITSET ***masksStarred;     // The same, but for short tables
    BITSET   *mask, *tmp;       // Buffers
    BITSET    lastWord0Then1;   // last word
    BITSET    lastWord1Then0;   //

    vec<BITSET>         stackedWords;     // the modified words
    vec<int>            stackedIndexes;   // stores the indexes of the words that have been stacked
    vec<pair>           stackStructure;   // stores, d the depth where nb words have been stacked
    vec<bool>           modifiedWords;    // indicates if the word has been modified
    vec<int>            deltaSizes;       // deltaSizes[x] indicates how many values are in the delta set of x
    SparseSetMultiLevel nonZeros;         // Words that are not zero
    bool                starred;          // IS a starred table
    bool                firstCall;        // First filtering call?
    vec<vec<int>>       residues;   // residues[x][a] is the index of the word where a support was found the last time for (x,a)
    uint64_t            lastTimestamps;   // last time it was called

    // Bitset methods
    void fillTo1(BITSET *bitset) const {
        for(int i = 0; i < nWords; i++) bitset[i] = ONLYONE;
        bitset[nWords - 1] = lastWord1Then0;
    }

    void fillTo0(BITSET *bitset, bool except_end = true) const {
        for(int i = 0; i < nWords; i++) bitset[i] = BZERO;
        if(except_end)
            bitset[nWords - 1] = lastWord0Then1;
    }

    void clearMask() { fillTo0(mask); }

    void addToMask(const BITSET *m) { make_or(mask, m); }

    void intersectWithMask();


    void make_or(BITSET *target, const BITSET *m) {
        for(int idx : nonZeros) target[idx] |= m[idx];
    }

    void addInverseToMask(const BITSET *m) {
        for(int idx : nonZeros) mask[idx] |= (~m[idx]);
    }

    int firstNonNullIntersectionIndex(const BITSET *t1, const BITSET *t2) {
        for(int idx : nonZeros)
            if((t1[idx] & t2[idx]) != BZERO)
                return idx;
        return -1;
    }


    void updateDomains();
    void beforeFiltering();
    void manageLastPastVar();
    void wordModified(int id, BITSET oldValue);
    void createMemoryForMask(BITSET ***themask) {
        themask = new BITSET **[scope.size()];
        for(int i = 0; i < scope.size(); i++) {
            themask[i] = new BITSET *[scope[i]->domain.maxSize()];
            for(int j = 0; j < scope[i]->domain.maxSize(); j++) {
                themask[i][j] = new BITSET[nWords];
                fillTo0(themask[i][j], false);
            }
        }
    }

   public:
    // Constructors
    CompactTable(Problem &p, std::string n, vec<Variable *> &vars, size_t max_n_tuples);
    CompactTable(Problem &p, std::string n, vec<Variable *> &vars, Matrix *tuplesFromOtherConstraint);
    void delayedConstruction(int id) override;
    bool onFirstCall();

    // filtering
    bool filter(Variable *x) override;
    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;

    // Notifications : restore validTuples when backtrack is performed
    void notifyDeleteDecision(Variable *x, int v, Solver &s) override;
    void attachSolver(Solver *s) override;


    // Debug methods
    void displaySupports();
    void displayCurrent(BITSET *tmp);
};
}   // namespace Cosoco


#endif   // COSOCO_COMPACTTABLE_H
