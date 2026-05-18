//
// Created by audemard on 18/06/2021.
//

#ifndef COSOCO_NOOVERLAP_H
#define COSOCO_NOOVERLAP_H


#include <constraints/globals/GlobalConstraint.h>
#include <core/Problem.h>

#include <set>

namespace Cosoco {

class NoOverlap : public GlobalConstraint {
    vec<Variable *> xs;
    vec<int>        widths;
    vec<Variable *> ys;
    vec<int>        heights;
    int             half;
    std::set<int>   overlappings;

    vec<vec<int> > residues1;
    vec<vec<int> > residues2;


   public:
    NoOverlap(Problem &pb, std::string &n, vec<Variable *> &xs, vec<int> &widths, vec<Variable *> &ys, vec<int> &heights);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;

    bool isCorrectlyDefined() override;

    bool overlap(int a, Variable *x, int b) { return a > x->maximum() && x->minimum() > b; }

   protected:
    bool filter(vec<Variable *> &x1, vec<int> &t1, vec<Variable *> &x2, vec<int> &t2, vec<vec<int> > &residues);
    bool findSupport(vec<Variable *> &x1, vec<int> &t1, vec<Variable *> &x2, vec<int> &t2, int w, int ww);
};
}   // namespace Cosoco

#endif   // COSOCO_NOOVERLAP_H
