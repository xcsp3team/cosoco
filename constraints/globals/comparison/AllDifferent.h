#ifndef ALLDIFFERENT_H
#define ALLDIFFERENT_H


#include "constraints/globals/GlobalConstraint.h"

// Using bound consistency : A fast and simple algorithm for bounds consistency of the alldifferent constraint
// Ijcai 2003 Alejandro Lopez-Ortiz, Claude-Guy Quimper, John Tromp, Peter van Beek
// Based on choco implementation


namespace Cosoco {

// ------------    Internal Data structures -----------------
class Interval {
   public:
    Interval(Variable *xx) : x(xx) { }


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

class AllDifferent : public GlobalConstraint {
   public:
    AllDifferent(Problem &p, std::string n, vec<Variable *> &vars);

    // Filtering method, return false if a conflict occurs
    virtual bool filter(Variable *x) override;

    // Checking
    virtual bool isSatisfiedBy(vec<int> &tuple) override;
    void         addInitialSATClauses() override;

    int variableConsistency;   // Use basic filtering limited to variable assignment
   protected:
    vec<int> t;   // Tree links
    vec<int> d;   // Diffs between critical capacities
    vec<int> h;   // Hall interval links
    vec<int> bounds;

    int nbBounds;

    vec<Interval *> interval;
    vec<Interval *> minsorted;
    vec<Interval *> maxsorted;

    bool basicFilter(Variable *x);

    void sortIt();
    bool filterLower(bool &again);
    bool filterUpper(bool &again);

    void pathset(vec<int> &tab, int start, int end, int to);
    int  pathmin(vec<int> &tab, int i);
    int  pathmax(vec<int> &tab, int i);
};

}   // namespace Cosoco


#endif /* ALLDIFFERENT_H */
