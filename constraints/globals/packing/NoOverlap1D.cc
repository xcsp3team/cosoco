//
// Created by audemard on 14/11/2026.
//

#include "NoOverlap1D.h"

#include "Solver.h"
//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool Cosoco::NoOverlap1D::isSatisfiedBy(vec<int> &tuple) {
    for(int i = 0; i < tuple.size(); i++) {
        for(int j = i + 1; j < tuple.size(); j++) {
            if((tuple[i] + lengths[i] <= tuple[j] || tuple[j] + lengths[j] <= tuple[i]) == false)
                return false;
        }
    }
    return true;
}

bool Cosoco::NoOverlap1D::isCorrectlyDefined() {
    if(scope.size() != lengths.size())
        return false;
}

//----------------------------------------------
// Filrtering
//----------------------------------------------
bool Cosoco::NoOverlap1D::timeTabling() {
    bool                 changed;
    std::vector<Segment> profiles;
    do {
        changed = false;

        // 1. Mandatory parts
        profiles.clear();
        for(int i = 0; i < scope.size(); i++) {
            if(lengths[i] == 0)
                continue;   // Task with length 0: ignored it
            int last                  = scope[i]->maximum();
            int earlierCompletionTime = scope[i]->minimum() + lengths[i];
            if(last < earlierCompletionTime)
                profiles.push_back({last, earlierCompletionTime, i});
        }
        std::sort(profiles.begin(), profiles.end(), [](const Segment &a, const Segment &b) { return a.start < b.start; });

        // 2. Pfofiles consistency (capacity 1)
        for(size_t k = 1; k < profiles.size(); k++)
            if(profiles[k - 1].end > profiles[k].start)
                return false;

        // 3. Filtering
        for(int j = 0; j < scope.size(); j++) {
            int length = lengths[j];
            if(length == 0)
                continue;

            Variable *x      = scope[j];
            int       oldMin = x->minimum(), oldMax = x->maximum();

            // First Segment that end after minX (profiles are sorted)
            size_t k =
                std::upper_bound(profiles.begin(), profiles.end(), oldMin, [](int v, const Segment &g) { return v < g.end; }) -
                profiles.begin();

            for(; k < profiles.size(); k++) {
                const Segment &g = profiles[k];
                if(g.start - length >= x->maximum())
                    break;   // all remaining segments are on the right
                if(g.id == j)
                    continue;   // Nothing to do against itself

                int a = std::max(g.start - length + 1, x->minimum());
                int b = std::min(g.end - 1, x->maximum());
                if(a > b)
                    continue;
                for(int v = a; v <= b; v++)   // remove values in [a,b]
                    if(solver->delVal(x, v) == false)
                        return false;
            }

            // Fix point depends on bounds
            if(x->minimum() != oldMin || x->maximum() != oldMax)
                changed = true;
        }
    } while(changed);
    return true;
}


bool Cosoco::NoOverlap1D::filter(Variable *x) { return timeTabling(); }

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

Cosoco::NoOverlap1D::NoOverlap1D(Problem &p, vec<Variable *> &vars, vec<int> &_lengths)
    : GlobalConstraint(p, "NoOverlap1D", vars) {
    _lengths.copyTo(lengths);
    isPostponable = true;
}
