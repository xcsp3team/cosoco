#ifndef ALLDIFFERENT_H
#define ALLDIFFERENT_H


#include <Matcher.h>

#include "constraints/globals/GlobalConstraint.h"


namespace Cosoco {


//----------------------------------------------------------

class AllDifferent : public GlobalConstraint {
   public:
    AllDifferent(Problem &p, std::string n, vec<Variable *> &vars);

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

class AllDifferentWeak : public AllDifferent {
   public:
    AllDifferentWeak(Problem &p, std::string n, vec<Variable *> &vars);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;
};

class AllDifferentAC : public AllDifferent {
    Matcher *matcher;

   public:
    AllDifferentAC(Problem &p, std::string n, vec<Variable *> &vars);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    void attachSolver(Solver *s) override;
};

}   // namespace Cosoco


#endif /* ALLDIFFERENT_H */
