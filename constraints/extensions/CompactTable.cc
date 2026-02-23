//
// Created by audemard on 30/04/24.
//

#include "CompactTable.h"

#include <solver/Solver.h>

#include <bitset>
#include <cmath>
using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool CompactTable::isSatisfiedBy(vec<int> &tuple) {
    for(unsigned int i = 0; i < tuples->nrows(); i++) {
        int nb = 0;
        for(int j = 0; j < scope.size(); j++) {
            if((*tuples)[i][j] != STAR && tuple[j] != scope[j]->domain.toVal((*tuples)[i][j]))
                break;
            nb++;
        }
        if(nb == scope.size())
            return true;
    }
    return false;
}


//----------------------------------------------
// Main filtering method
//----------------------------------------------
bool CompactTable::filter(Variable *dummy) {
    if(firstCall && onFirstCall() == false)
        return false;
    beforeFiltering();
    // std::cout << "\nbefore\n";
    // displayCurrent(current);
    //   we compute in mask the bit vector denoting all deleted tuples (and then we inverse it)
    clearMask();
    for(int posx : SVal) {
        Variable *x = scope[posx];
        // std::cout << "modify mask for " << x->_name << " " << x->size() << std::endl;
        if(0 && x->size() > 1 && deltaSizes[posx] <= x->size()) {
            for(int cnt = deltaSizes[posx] - 1, idv = x->domain.lastRemoved(); cnt >= 0; cnt--) {
                addToMask(!starred ? masks[posx][idv] : masksStarred[posx][idv]);
                idv = x->domain.prevRemoved(idv);
            }
        } else if(x->size() == 1) {
            addInverseToMask(masks[posx][x->domain[0]]);
        } else {
            fillTo0(tmp);
            for(int idv : x->domain) make_or(tmp, masks[posx][idv]);
            addInverseToMask(tmp);
        }
        // displayCurrent(mask);
    }
    intersectWithMask();   // Update current table
    // displayCurrent(current);

    if(nonZeros.size() == 0)
        return false;   // inconsistency detected

    updateDomains();   //
    return true;
}

void CompactTable::beforeFiltering() {
    if(lastTimestamps == 0 || lastTimestamps != solver->globalTimestamps()) {
        for(int id : nonZeros) modifiedWords[id] = false;
        if(stackedWords.size() > 0 && stackStructure.last().level == solver->decisionLevel())
            for(int i = stackStructure.last().nb - 1; i >= 0; i--)
                modifiedWords[stackedIndexes[stackedIndexes.size() - i - 1]] = true;

        lastTimestamps = solver->globalTimestamps();
    }
    SSup.clear();
    SVal.clear();
    manageLastPastVar();
    for(int posx : unassignedVariablesIdx) {
        Variable *x = scope[posx];
        if(lastSizes[posx] != x->size()) {
            deltaSizes[posx] = lastSizes[posx] - x->size();
            SVal.push(posx);
            lastSizes[posx] = x->size();
        }
        if(x->size() > 1)
            SSup.push(posx);
    }
    if(SVal.size() == 1)
        SSup.remove(SVal[0]);
    // std::cout << "sval szie:" << SVal.size() << " SSup size " << SSup.size() << "\n";
}

void CompactTable::manageLastPastVar() {
    int posLast = NOTINSCOPE;
    if(solver->decisionLevel() > 0) {
        Variable *last = solver->decisionVariableAtLevel(solver->decisionLevel());
        posLast        = toScopePosition(last->idx);
        if(posLast != NOTINSCOPE && lastSizes[posLast] != 1) {
            deltaSizes[posLast] = lastSizes[posLast] - 1;
            SVal.push(posLast);
            lastSizes[posLast] = 1;
        }
    }
}


void CompactTable::updateDomains() {   // we update domains (inconsistency is no more possible)
    for(int posx : SSup) {
        Variable *x = scope[posx];
        for(int idv : x->domain) {
            int r = residues[posx][idv];
            if((current[r] & masks[posx][idv][r]) != BIT_ZERO)   //  residue is still ok
                continue;
            r = firstNonNullIntersectionIndex(current, masks[posx][idv]);
            if(r != -1) {
                residues[posx][idv] = r;
            } else {
                solver->delIdv(x, idv);
                // return false;
            }
        }
        lastSizes[posx] = x->size();
        assert(x->size() > 0);
    }
}

void CompactTable::wordModified(int index, BITSET oldValue) {
    if(modifiedWords[index]) {   // Already done
        assert(stackStructure.last().level == solver->decisionLevel());
        return;
    }
    int level = solver->decisionLevel();
    if(stackedWords.size() == 0 || stackStructure.last().level != level)   // First time a word is modified in the level
        stackStructure.push({level, 1});
    else
        stackStructure.last().nb++;   // another modified word at this level
    stackedWords.push(oldValue);
    stackedIndexes.push(index);
    modifiedWords[index] = true;
}


void CompactTable::notifyDeleteDecision(Variable *x, int v, Solver &s) {
    // std::cout << "stacks\n";
    // for(auto &s : stackStructure) std::cout << "(level:" << s.level << ",nb:" << s.nb << ")";
    // std::cout << std::endl;
    if(stackedWords.size() > 0 && stackStructure.last().level == s.decisionLevel() + 1) {
        for(int i = stackStructure.last().nb - 1; i >= 0; i--) {
            current[stackedIndexes.last()] = stackedWords.last();
            stackedIndexes.pop();
            stackedWords.pop();
        }
        stackStructure.pop();
    }
    if(nonZeros.isLimitRecordedAtLevel(s.decisionLevel() + 1))
        nonZeros.restoreLimit(s.decisionLevel() + 1);
    lastTimestamps = 0;
    lastSizes.fill(0);
}

void CompactTable::intersectWithMask() {
    int level = solver->decisionLevel();
    for(int id : reverse(nonZeros)) {
        BITSET l = current[id] & ~mask[id];   // we inverse mask
        if(current[id] != l) {
            wordModified(id, current[id]);
            // std::cout << id << "=> " << std::bitset<SIZEW>(l) << "\n";
            // std::cout << "c=> " << std::bitset<SIZEW>(current[id]) << "\n";
            // std::cout << "m=> " << std::bitset<SIZEW>(~mask[id]) << "\n";
            current[id] = l;

            if(l == BIT_ZERO) {
                if(nonZeros.isLimitRecordedAtLevel(level) == false)
                    nonZeros.recordLimit(level);
                nonZeros.del(id);
            }
        }
    }
}
//----------------------------------------------
// Constructors and initialisation
//----------------------------------------------

CompactTable::CompactTable(Cosoco::Problem &p, std::string n, vec<Cosoco::Variable *> &vars, size_t max_n_tuples)
    : Extension(p, n, vars, max_n_tuples, true), nonZeros() {
    type = "Extension - CT";
}


CompactTable::CompactTable(Problem &p, std::string n, vec<Variable *> &vars, Matrix<int> *tuplesFromOtherConstraint)
    : Extension(p, n, vars, true, tuplesFromOtherConstraint), nonZeros() {
    type = "Extension - CT";
}


void CompactTable::delayedConstruction(int id) {
    Constraint::delayedConstruction(id);
    // Create space
    nWords         = (int)ceil(((double)tuples->nrows()) / ((double)SIZEW));
    current        = new BITSET[nWords];
    mask           = new BITSET[nWords];
    tmp            = new BITSET[nWords];
    lastWord1Then0 = tuples->nrows() % SIZEW != 0 ? ONETO(tuples->nrows() % 64) : BIT_ALL_ONE;
    lastWord0Then1 = tuples->nrows() % SIZEW != 0 ? ZEROTO(tuples->nrows() % 64) : 0uLL;
    fillTo1(current);
    starred = tuples->starred;
    // createMemoryForMask(masks);
    masks = new BITSET **[scope.size()];
    for(int i = 0; i < scope.size(); i++) {
        masks[i] = new BITSET *[scope[i]->domain.maxSize()];
        for(int j = 0; j < scope[i]->domain.maxSize(); j++) {
            masks[i][j] = new BITSET[nWords];
            fillTo0(masks[i][j], false);
        }
    }

    // Create masks supports for each value for each variable
    if(starred == false) {
        for(size_t i = 0; i < tuples->nrows(); i++) {   // For each tuple
            auto idInCurrent = i / SIZEW;               // The word in current
            auto pos         = i % SIZEW;               // Bit position
            for(size_t posx = 0; posx < tuples->ncolumns(); posx++)
                masks[posx][(*tuples)[i][posx]][idInCurrent] |= SETBIT(pos);   // the idv for x is in this tuple
        }
    } else {
        for(int posx = 0; posx < scope.size(); posx++) {
            for(size_t i = 0; i < tuples->nrows(); i++) {
                auto idInCurrent = i / SIZEW;   // The word in current
                auto pos         = i % SIZEW;
                if((*tuples)[i][posx] != STAR)
                    masks[posx][(*tuples)[i][posx]][idInCurrent] |= SETBIT(pos);
                else {
                    for(int idv = 0; idv < scope[posx]->domain.maxSize(); idv++) masks[posx][idv][idInCurrent] |= SETBIT(pos);
                }
            }
            /*
           createMemoryForMask(masksStarred);
           for(int x = 0; x < scp.length; x++) {
               long[][] mask = masksS[x];
               for(int j = 0; j < tuples.length; j++)
                   if(tuples[j][x] != Constants.STAR)
                       Bit.setTo1(mask[tuples[j][x]], j);
           }
           */
        }
    }

    modifiedWords.growTo(nWords, false);

    deltaSizes.growTo(scope.size(), 0);
    nonZeros.setCapacity(nWords, true);
    residues.growTo(scope.size());
    for(int i = 0; i < residues.size(); i++) residues[i].growTo(scope[i]->domain.maxSize());
    firstCall = true;
    for(Variable *x : scope) lastSizes.push(x->size());
    lastTimestamps = 0;
    // displaySupports();
}


bool CompactTable::onFirstCall() {
    firstCall = false;
    int posx  = 0;
    for(Variable *x : scope) {
        for(int idv : x->domain) {
            int r = firstNonNullIntersectionIndex(current, masks[posx][idv]);
            if(r != -1)
                residues[posx][idv] = r;
            else {
                if(solver->delIdv(x, idv) == false)
                    return false;
            }
        }
        posx++;
    }
    return true;
}

void CompactTable::attachSolver(Solver *s) {
    Constraint::attachSolver(s);
    s->addObserverDeleteDecision(this);   // We need to restore validTuples.
}


// Debug methods

void CompactTable::displaySupports() {
    std::cout << "nW" << nWords << "\n";
    for(int i = 0; i < scope.size(); i++) {
        std::cout << "--------\n";
        for(int j = 0; j < scope[i]->domain.maxSize(); j++) {
            std::cout << "support for " << scope[i]->_name << "=" << scope[i]->domain.toVal(j) << "\n";
            for(int k = 0; k < nWords; k++) std::cout << std::bitset<SIZEW>(masks[i][j][k]) << "\n";
        }
    }
}

void CompactTable::displayCurrent(BITSET *tmp) {
    for(int i : nonZeros) std::cout << i << ": " << std::bitset<SIZEW>(tmp[i]) << "\n";
}