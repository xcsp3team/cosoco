//
// Created by audemard on 09/04/25.
//

#ifndef STR0_H
#define STR0_H
#include <Extension.h>
#include <ObserverDecision.h>


namespace Cosoco {
class STR0 : public Extension, ObserverDeleteDecision {
   protected:
    SparseSet       set;        // The current tuples of the table
    vec<vec<bool> > ac;         // ac[x][a] indicates if a support has been found for (x,a)
    int            *cnts;       // cnts[x] is the number of values in the current domain of x with no found support (yet)
    int             sSupSize;   // The number of variables for which support searching must be done
    int            *sSup;       //  The (dense) set of positions of variables for which support searching must be done.

    bool universal = false;
    void beforeFiltering();
    bool isValidTuple(int *t);
    bool updateDomains();

   public:
    STR0(Problem &p, std::string n, vec<Variable *> &vars, size_t max_n_tuples);
    STR0(Problem &p, std::string n, vec<Variable *> &vars, Matrix<int> *tuplesFromOtherConstraint);

    // filtering
    bool filter(Variable *x) override;
    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;

    // Notifications : restore validTuples when backtrack is performed
    void notifyDeleteDecision(Variable *x, int v, Solver &s) override;

    void delayedConstruction(int id) override;
    void attachSolver(Solver *s) override;
    bool isCorrectlyDefined() override;
};
}   // namespace Cosoco


#endif   // STR0_H
