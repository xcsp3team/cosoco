//
// Created by audemard on 18/06/2021.
//

#include "NoOverlap.h"

#include <mtl/Sort.h>

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool NoOverlap::isSatisfiedBy(vec<int> &tuple) {
    for(int i = 0; i < half; i++)
        for(int j = i + 1; j < half; j++) {
            int xi = tuple[i], xj = tuple[j], yi = tuple[i + half], yj = tuple[j + half];
            if(!(xi + widths[i] <= xj || xj + widths[j] <= xi || yi + heights[i] <= yj || yj + heights[j] <= yi))
                return false;
        }

    return true;
}


bool NoOverlap::isCorrectlyDefined() { return true; }


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool NoOverlap::filter(Variable *dummy) { return filter(xs, widths, ys, heights) && filter(ys, heights, xs, widths); }


// optimizations are possible ; to be done
bool NoOverlap::filter(vec<Variable *> &x1, vec<int> &t1, vec<Variable *> &x2, vec<int> &t2) {
    for(int i = 0; i < half; i++) {
        Variable *x11 = x1[i];
        for(int idv : x11->domain) {
            bool lextern = false;
            int v = x11->domain.toVal(idv);   // we are going to look for a support of (x1[i],v)
            // we compute the set of tasks overlapping on the first axis wrt (x1[i],v)
            overlappings.clear();
            for(int j = 0; j < half; j++)
                if(j != i && overlap(v + t1[i], x1[j], v - t1[j]))
                    overlappings.insert(j);
            if(overlappings.empty())
                continue;
            if(overlappings.size() == 1) {
                int j = *overlappings.begin();
                if(!overlap(x2[i]->minimum() + t2[i], x2[j], x2[i]->maximum() - t2[j]))
                    continue;
                // otherwise it means that overlapping is present on both dimensions (so, there is no support for (x1[i],v))
            } else {
                // we now look for a value w in the domain of x2[i] that is compatible with first axis overlapping boxes
                // a kind of k-wise consistency is used (see paper about sweep for information about the principle)
                // also, a local form of energetic reasoning is used
                Variable *x22 = x2[i];
                for(int idv2 : x22->domain) {
                    bool lintern = false;
                    long volume = 0;
                    int  minX = INT_MAX, minY = INT_MAX;
                    int  maxX = INT_MIN, maxY = INT_MIN;
                    int  w = x22->domain.toVal(idv2);
                    for(int j : overlappings) {
                        if(overlap(w + t2[i], x2[j], w - t2[j])) {
                            lintern = true;
                            break;   // to try another value w
                        }
                        minX = std::min(minX, x1[j]->minimum());
                        minY = std::min(minY, x2[j]->minimum());
                        maxX = std::max(maxX, x1[j]->maximum() + t1[j]);
                        maxY = std::max(maxY, x2[j]->maximum() + t2[j]);
                        volume += t1[j] * t2[j];
                    }
                    if(lintern)
                        continue;
                    int diffX = maxX - minX + 1, diffY = maxY - minY + 1;
                    // we can remove up to t2[i] at diffY because there may be no possible overlapping on x along this height
                    if(w < minY && minY < w + t2[i])
                        diffY -= std::min(maxY, w + t2[i]) - minY;
                    else if(minY <= w && w < maxY)
                        diffY -= std::min(maxY, w + t2[i]) - w;
                    if(volume > diffX * diffY)    // not enough room for the items
                       continue;           // to try another value w

                    lextern = true;  // because found support
                    break;
                }
                if(lextern)
                    continue;
            }
            // at this step, no support has been found
            if(solver->delVal(x11, v) == false)
                return false;
        }
    }
    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------


NoOverlap::NoOverlap(Problem &pb, std::string &n, vec<Variable *> &X, vec<int> &w, vec<Variable *> &Y, vec<int> &h)
    : GlobalConstraint(pb, n, "NoOverlap", 2 * X.size()) {
    X.copyTo(xs);
    Y.copyTo(ys);
    vec<Variable *> vars;
    for(Variable *x : xs) vars.push(x);
    for(Variable *y : ys) vars.push(y);
    scopeInitialisation(vars);
    w.copyTo(widths);
    h.copyTo(heights);
    half = xs.size();
}
