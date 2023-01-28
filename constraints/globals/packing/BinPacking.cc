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
    for(int j = 0; j < usableBins.size(); j++) {
        int i = usableBins[j];
        bins[i]->set(i, limits[i]);   // the bin is reinitialized with its initial capacity
        sortedBins[j] = bins[i];
    }

    // updating the capacity of bins
    for(int posx = 0; posx < nItems; posx++)
        if(scope[posx]->size() == 1)
            bins[scope[posx]->domain[0]]->capacity -= sizes[posx];   // the capacity is updated

    // putting each object in front of the right bin (the first one where it can enter)
    int maxSize   = -1;
    int sortLimit = usableBins.size();
start:
    while(true) {
        maxSize = -1;
        sort(sortedBins, sortLimit, CompareBins());

        if(sortedBins[0]->capacity < 0)
            return false;   // TODO 1: moving it earlier (avoid the first sort) ?

        for(SparseSet &set : fronts)   // TODO 2: only clearing from 0 to usableBins.limit ?
            set.clear();
        for(int posx = 0; posx < nItems; posx++) {   // TODO 3: only iterating over future variables? (but the  condition remains)
            Variable *x = scope[posx];
            if(x->size() == 1)
                continue;   // because already considered (when reducing the appropriate bin capacity)
            int position = -1;
            for(int j = 0; position == -1 && j < usableBins.size(); j++) {
                int i = sortedBins[j]->index;
                if(sizes[posx] > sortedBins[j]->capacity) {
                    if(solver->delIdv(x, i) == false)
                        return false;
                } else if(x->containsIdv(i)) {
                    position = j;
                    fronts[j].add(posx);
                }
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
            if(sizes[posx] > maxSize)
                maxSize = sizes[posx];
        }
        break;
    }

    bool energetic = true;
    if(energetic) {
        // energetic reasoning
        int cumulatedCapacities = 0, cumulatedSizes = 0;
        for(int j = usableBins.size() - 1; j >= 0; j--) {
            int capacity = sortedBins[j]->capacity;
            cumulatedCapacities += capacity;
            if(fronts[j].size() == 0)
                continue;
            int min = INT_MAX, max = -1;
            for(int k = 0; k < fronts[j].size(); k++) {
                int size = sizes[fronts[j][k]];
                min      = std::min(min, size);
                max      = std::max(max, size);
                cumulatedSizes += size;
            }

            bool onyOnePossibleInTheBin = min > capacity / 2;
            sortedBins[j]->lost         = onyOnePossibleInTheBin ? capacity - max : 0;   // local j-lost place
            int lost                    = sortedBins[j]->lost;
            // under certain conditions, we can combine several local lost places
            for(int jj = usableBins.size() - 1; jj > j; jj--) {
                if(min <= sortedBins[jj]->lost)
                    sortedBins[jj]->lost = 0;
                else
                    lost += sortedBins[jj]->lost;
            }
            // note that even if several bins have the same current capacity, it does not mean that all items
            // are in front of the leftmost one (due to other constraints)
            // margin is a global value computed when reasoning from the jth sorted bin to the rightmost one
            int margin = cumulatedCapacities - lost - cumulatedSizes;

            // the margin is computed from the object of max size (when only one possible)

            bool firstPart = true;
            if(firstPart) {
                if(margin < 0)
                    return false;
                if(onyOnePossibleInTheBin && margin - (max - min) < 0) {
                    // we can remove some values if the smallest item cannot be put in the bin j
                    for(int k = 0; k < fronts[j].size(); k++) {
                        int posx = fronts[j][k];
                        if(margin - (max - sizes[posx]) < 0 && solver->delIdv(scope[posx], sortedBins[j]->index) == false)
                            // scp[i].dom.removeValueIfPresent(sortedBins[j].index) == false)
                            return false;
                    }
                }
            }
            // maybe, some items in front of a bin on the left have a size greater than the margin (we can then
            // remove them from bins on the right)
            bool additionalFiltering = true;
            if(additionalFiltering)
                if(maxSize > margin) {
                    for(int left = 0; left < j; left++) {
                        if(fronts[left].size() == 0)
                            continue;
                        for(int k = 0; k < fronts[left].size(); k++) {
                            int p    = fronts[left][k];
                            int size = sizes[p];
                            if(size <= margin)
                                continue;
                            for(int right = usableBins.size() - 1; right >= j; right--) {
                                if(solver->delIdv(scope[p], sortedBins[right]->index) == false)
                                    return false;
                            }
                        }
                    }
                }
        }
    }

    // we look for the index of the smallest free item, and also compute the min and max usable bin numbers
    int smallestFreeItem = -1, minUsableBin = INT_MAX, maxUsableBin = -1;
    for(int i = 0; i < nItems; i++) {
        Variable *x = scope[i];
        if(x->size() > 1) {
            if(smallestFreeItem == -1 || sizes[i] < sizes[smallestFreeItem])
                smallestFreeItem = i;
            minUsableBin = std::min(minUsableBin, x->minimum());
            maxUsableBin = std::max(maxUsableBin, x->maximum());
        }
    }

    if(smallestFreeItem == -1)
        return true;

    // we discard bins that are now identified as useless because we cannot even put the smallest item in it
    for(int j = usableBins.size() - 1; j >= 0; j--) {
        // for breaking, we should go from 0 to ..., but removing an element in usableBins could be a problem
        int i = sortedBins[j]->index;
        assert(usableBins.contains(i));
        if(sortedBins[j]->capacity < sizes[smallestFreeItem]) {   // || i < minUsableBin || i > maxUsableBin)
            if(usableBins.isLimitRecordedAtLevel(solver->decisionLevel()) == false)
                usableBins.recordLimit(solver->decisionLevel());
            usableBins.del(i);
        }
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