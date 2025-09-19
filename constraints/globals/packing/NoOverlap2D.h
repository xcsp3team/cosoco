//
// Created by audemard on 18/09/2025.
//

#ifndef COSOCO_NOOVERLAP2D_H
#define COSOCO_NOOVERLAP2D_H
#include <constraints/globals/GlobalConstraint.h>


namespace Cosoco {

class Graph {
   public:
    int            nbNodes;
    vec<SparseSet> edges;

    void setCapacity(int n) {
        nbNodes = n;
        edges.growTo(n);
        for(int i = 0; i < n; i++) edges[i].setCapacity(nbNodes, false);
    }
    void addEdge(int node1, int node2) {
        edges[node1].add(node2);
        edges[node2].add(node1);
    }
    void removeEdge(int node1, int node2) {
        edges[node1].del(node2);
        edges[node2].del(node1);
    }
};

#define CONFLICT -1
#define USELESS  0
#define PROP     1

class NoOverlap2D : public GlobalConstraint {
    vec<Variable *> x, y;
    vec<Variable *> dx, dy;
    int             n;
    vec<int>        boxesToCompute, pruneList;
    Graph           overlappingBoxes;


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
    bool boxInstantiated(int i);
    bool doOverlap(int i, int j, bool hori);
    int  prune(int j);
    int  doFiltering(int i, int j, bool hori);
    void prop(int idx);
    bool energyCheck(int i);
};
}   // namespace Cosoco


#endif   // COSOCO_NOOVERLAP2D_H
