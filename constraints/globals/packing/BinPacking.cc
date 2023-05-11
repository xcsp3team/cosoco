//
// Created by audemard on 08/02/2022.
//

#include "BinPacking.h"

#include "Sort.h"
#include "solver/Solver.h"

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool BinPacking::isSatisfiedBy(vec<int> &tuple) {
    sums.fill(0);
    for(int i = 0; i < tuple.size(); i++) sums[tuple[i]] += sizes[i];
    for(int i = 0; i < sums.size(); i++)
        if(sums[i] > limits[i])
            return false;
    return true;
}


bool BinPacking::isCorrectlyDefined() {
    for(int i = 0; i < nItems; i++)
        if(scope[i]->domain.isIndexesAreValues() == false)
            return false;
    return true;
}

//----------------------------------------------
// Filrtering
//----------------------------------------------
void BinPacking::notifyDeleteDecision(Variable *x, int v, Solver &s) {
    if(usableBins.isLimitRecordedAtLevel(s.decisionLevel() + 1))
        usableBins.restoreLimit(s.decisionLevel() + 1);
}


bool BinPacking::filter(Variable *dummy) {
    // initialization
    for(int i = 0; i < bins.size(); i++) bins[i]->set(i, limits[i]);   // the bin is reinitialized with its initial capacity

    // updating the capacity of bins
    for(int posx = 0; posx < nItems; posx++)
        if(scope[posx]->size() == 1) {
            bins[scope[posx]->domain[0]]->capacity -= sizes[posx];   // the capacity is updated
            if(bins[scope[posx]->domain[0]]->capacity < 0)
                return false;
        }

start:
    while(true) {
        for(int posx = 0; posx < nItems; posx++) {   // TODO 3: only iterating over future variables? (but the  condition remains)
            Variable *x = scope[posx];
            if(x->size() == 1)
                continue;   // because already considered (when reducing the appropriate bin capacity)
            int position = -1;
            for(int j = 0; position == -1 && j < usableBins.size(); j++) {
                int i = usableBins[j];
                if(sizes[posx] > bins[i]->capacity) {
                    if(solver->delIdv(x, i) == false)
                        return false;
                } else if(x->containsIdv(i))
                    position = j;
            }
            if(position == -1)
                return false;

            if(x->size() == 1) {
                bins[x->domain[0]]->capacity -= sizes[posx];   // note that sortedBins has references to bins
                if(bins[x->domain[0]]->capacity < 0)
                    return false;
                // sortLimit = position + 1;   // TODO only inserting rather than sorting ?
                goto start;
            }
        }
        break;
    }

    bool energetic = true;
    if(energetic) {
        int cumulatedCapacities = 0, cumulatedSizes = 0, lost = 0;
        for(int j = usableBins.size() - 1; j >= 0; j--) {
            Bin *b        = bins[usableBins[j]];
            b->minSizeObj = INT_MAX;
            b->maxSizeObj = -1;
            for(int posx = 0; posx < nItems; posx++) {
                if(scope[posx]->size() == 1)
                    continue;   // because already considered (when reducing the appropriate bin capacity)
                if(scope[posx]->containsValue(b->index)) {
                    b->minSizeObj = std::min(b->minSizeObj, sizes[posx]);
                    b->maxSizeObj = std::max(b->maxSizeObj, sizes[posx]);
                }
            }
            if(b->maxSizeObj != -1) {                 // the bin remains usable (as at least an object can enter it)
                cumulatedCapacities += b->capacity;
                if(b->minSizeObj > b->capacity / 2)   // if only one object can enter
                    lost += b->capacity - b->maxSizeObj;
            }
            // System.out.println(" bin " + b.index + " " + b.minSizeObj + " " + b.maxSizeObj);
        }
        int available = cumulatedCapacities - lost;
        for(int posx = 0; posx < nItems; posx++) {
            if(scope[posx]->size() > 1)
                cumulatedSizes += sizes[posx];
        }
        if(available < cumulatedSizes)
            return false;
        for(int j = usableBins.size() - 1; j >= 0; j--) {
            Bin *b = bins[usableBins[j]];
            if(b->maxSizeObj == -1)                            // no more useful bin
                continue;
            if(b->minSizeObj > b->capacity / 2)                // a form of lost space already computed (don't count it twice)
                continue;
            if(b->minSizeObj + b->maxSizeObj <= b->capacity)   // no hope to remove any value (with our reasoning)
                continue;
            for(int posx = 0; posx < nItems; posx++) {
                if(scope[posx]->size() != 1 && scope[posx]->containsIdv(b->index) && sizes[posx] != b->minSizeObj &&
                   b->minSizeObj + sizes[posx] > b->capacity) {
                    int relost = b->capacity - sizes[posx];
                    if(available - relost < cumulatedSizes) {
                        if(solver->delIdv(scope[posx], b->index) == false)
                            return false;
                    }
                }
            }
        }
    }
    // we look for the index of the smallest free item, and also compute the min and max usable bin numbers
    int smallestFreeItem = -1, minUsableBin = INT_MAX, maxUsableBin = -1;
    for(int posx = 0; posx < nItems; posx++) {
        Variable *x = scope[posx];
        if(x->size() > 1) {
            if(smallestFreeItem == -1 || sizes[posx] < sizes[smallestFreeItem])
                smallestFreeItem = posx;
            minUsableBin = std::min(minUsableBin, x->minimum());
            maxUsableBin = std::max(maxUsableBin, x->maximum());
        }
    }
    if(smallestFreeItem == -1)
        return true;
    // we discard bins that are now identified as useless because we cannot even put the smallest item in it
    for(int j = usableBins.size() - 1; j >= 0; j--) {
        // for breaking, we should go from 0 to ..., but removing an element in usableBins could be a problem
        Bin *b = bins[usableBins[j]];
        assert(usableBins.contains(b->index));
        if(b->maxSizeObj == -1 || b->capacity < sizes[smallestFreeItem] || b->index < minUsableBin || maxUsableBin < b->index)
            if(usableBins.isLimitRecordedAtLevel(solver->decisionLevel()) == false)
                usableBins.recordLimit(solver->decisionLevel());
        usableBins.del(b->index);
    }
    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

void BinPacking::attachSolver(Solver *s) {
    Constraint::attachSolver(s);
    s->addObserverDeleteDecision(this);   // We need to restore validTuples.
}


void BinPacking::delayedConstruction(int id) {
    Constraint::delayedConstruction(id);
    usableBins.setCapacity(bins.size(), true);
    for(SparseSet &s : fronts) s.setCapacity(sizes.size(), true);
}


BinPacking::BinPacking(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &_sizes, vec<int> &_limits)
    : GlobalConstraint(p, n, "BinPacking", vars) {
    nItems = _sizes.size();
    nBins  = _limits.size();
    _sizes.copyTo(sizes);
    _limits.copyTo(limits);
    sums.growTo(nBins, 0);
    for(int i = 0; i < nBins; i++) bins.push(new Bin);
    bins.copyTo(sortedBins);
    fronts.growTo(nBins);
}