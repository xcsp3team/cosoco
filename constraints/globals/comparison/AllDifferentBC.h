#ifndef ALLDIFFERENTBC_H
#define ALLDIFFERENTBC_H


#include "AllDifferent.h"
#include "Variable.h"

// Using bound consistency : A fast and simple algorithm for bounds consistency of the alldifferent constraint
// Ijcai 2003 Alejandro Lopez-Ortiz, Claude-Guy Quimper, John Tromp, Peter van Beek
// Based on choco implementation


namespace Cosoco {

// ------------    Internal Data structures -----------------
class Interval {
   public:
    explicit Interval(Variable *xx) : x(xx) { }


    int       minrank, maxrank;
    Variable *x;
    int       lb, ub;
};

// ------------    Sort comparison -------------------------
struct MaxOrder {
    bool operator()(Interval *o1, Interval *o2) { return o1->ub < o2->ub; }
};

struct MinOrder {
    bool operator()(Interval *o1, Interval *o2) const { return o1->lb < o2->lb; }
};

//----------------------------------------------------------
class AllDifferentBC : public AllDifferent {
   public:
    AllDifferentBC(Problem &p, std::string n, vec<Variable *> &vars);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;


   protected:
    vec<int> t;   // Tree links
    vec<int> d;   // Diffs between critical capacities
    vec<int> h;   // Hall interval links
    vec<int> bounds;

    int nbBounds;

    vec<Interval *> interval;
    vec<Interval *> minsorted;
    vec<Interval *> maxsorted;


    void sortIt();
    bool filterLower(bool &again);
    bool filterUpper(bool &again);

    void pathset(vec<int> &tab, int start, int end, int to);
    int  pathmin(vec<int> &tab, int i);
    int  pathmax(vec<int> &tab, int i);
};

}   // namespace Cosoco


#endif /* ALLDIFFERENT_H */
