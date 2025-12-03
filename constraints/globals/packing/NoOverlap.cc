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

bool NoOverlap::filter(Variable *dummy) {
    return filter(xs, widths, ys, heights, residues1) && filter(ys, heights, xs, widths, residues2);
}


// optimizations are possible ; to be done
bool NoOverlap::filter(vec<Variable *> &x1, vec<int> &t1, vec<Variable *> &x2, vec<int> &t2, vec<vec<int> > &residues) {
    bool find = false;
    for(int i = 0; i < half; i++) {
        Variable *dom1 = x1[i];
        for(int k = 0; k < dom1->domain.size(); k++) {
            find    = false;
            int idv = dom1->domain[k];
            int v   = dom1->domain.toVal(idv);   // we are going to look for a support of (x1[i],v)
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
                // otherwise it means that overlapping is present on both dimensions (so, there is no support for
                // (x1[i],v))
            } else {
                // we now look for a value w in the domain of x2[i] that is compatible with first axis overlapping
                // boxes
                // a kind of k-wise consistency is used (see paper about sweep for information about the principle)
                // also, a local form of energetic reasoning is used
                Variable *dom2    = x2[i];
                int       residue = residues[i][idv];
                if(residue != -1 && dom2->domain.containsIdv(residue)) {
                    int w = dom2->domain.toVal(residue);
                    if(findSupport(x1, t1, x2, t2, w, w + t2[i])) {
                        k = dom1->size();
                        continue;
                    }
                }
                for(int idv2 : dom2->domain) {
                    if(idv2 == residue)
                        continue;
                    int w = dom2->domain.toVal(idv2);
                    if(findSupport(x1, t1, x2, t2, w, w + t2[i])) {
                        residues[i][idv] = idv2;
                        find             = true;
                        break;
                    }
                }
            }
            // at this step, no support has been found
            if(find == false && solver->delIdv(dom1, idv) == false)
                return false;
        }
    }
    return true;
}


bool NoOverlap::findSupport(vec<Variable *> &x1, vec<int> &t1, vec<Variable *> &x2, vec<int> &t2, int w, int ww) {
    // ww = w + t2[i]
    long volume = 0;
    int  minX = INT_MAX, minY = INT_MAX;
    int  maxX = INT_MIN, maxY = INT_MIN;
    for(int j : overlappings) {
        if(overlap(ww, x2[j], w - t2[j]))
            return false;   // to try another value w
        minX = std::min(minX, x1[j]->minimum());
        minY = std::min(minY, x2[j]->minimum());
        maxX = std::max(maxX, x1[j]->maximum() + t1[j]);
        maxY = std::max(maxY, x2[j]->maximum() + t2[j]);
        volume += t1[j] * t2[j];
    }
    int diffX = maxX - minX + 1, diffY = maxY - minY + 1;
    // we can remove up to t2[i] at diffY because there may be no possible overlapping on x along
    // this height
    if(w < minY && minY < ww)
        diffY -= std::min(maxY, ww) - minY;
    else if(minY <= w && w < maxY)
        diffY -= std::min(maxY, ww) - w;
    if(volume > diffX * diffY)   // not enough room for the items
        return false;            // to try another value w
    return true;                 // because found support
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

NoOverlap::NoOverlap(Problem &pb, std::string &n, vec<Variable *> &X, vec<int> &w, vec<Variable *> &Y, vec<int> &h)
    : GlobalConstraint(pb, n, "NoOverlap", Constraint::createScopeVec(&X, &Y)) {
    X.copyTo(xs);
    Y.copyTo(ys);
    w.copyTo(widths);
    h.copyTo(heights);
    half = xs.size();
    residues1.growTo(xs.size());
    for(int i = 0; i < xs.size(); i++) residues1[i].growTo(xs[i]->domain.maxSize(), -1);

    residues2.growTo(ys.size());
    for(int i = 0; i < ys.size(); i++) residues2[i].growTo(ys[i]->domain.maxSize(), -1);
}
