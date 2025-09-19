//
// Created by audemard on 18/09/2025.
//

#ifndef COSOCO_NOOVERLAP2D_H
#define COSOCO_NOOVERLAP2D_H
#include <constraints/globals/GlobalConstraint.h>


namespace Cosoco {

class NoOverlap2D : public GlobalConstraint {
    vec<Variable *> x, y;
    vec<Variable *> dx, dy;
    int             n;
    vec<int>        boxesToCompute, pruneList;
    vec<vec<int> >  neighbours;


   public:
    NoOverlap2D(Problem &pb, std::string &n, vec<Variable *> &_x, vec<Variable *> &_y, vec<Variable *> &_dx,
                vec<Variable *> &_dy);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;


   protected:
    bool mayOverlap(int i, int j);
    bool isNotDisjoint(int i, int j, bool horizontal);
    bool doOverlap(int i, int j, bool hori);
    bool prune(int j);
    bool doFiltering(int i, int j, bool hori);
    void prop(int idx);
};
}   // namespace Cosoco


#endif   // COSOCO_NOOVERLAP2D_H
