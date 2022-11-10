#ifndef LEXICOGRAPHIC_H
#define LEXICOGRAPHIC_H

#include "constraints/globals/GlobalConstraint.h"

namespace Cosoco {

class Lexicographic : public GlobalConstraint {
   public:
    vec<Variable *> X, Y;
    bool            strict;
    Lexicographic(Problem &p, std::string n, vec<Variable *> &X, vec<Variable *> &Y, bool st);

    bool isCorrectlyDefined() override;


    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;

   protected:
    vec<int> times;   // times[posx] gives the time at which the variable x has been set (pseudo-assigned)
    vec<int> vals;    // vals[posx] gives the val of the variable x set at time times[posx]
    int      time;

    bool establishAC(Variable *x, Variable *y);
    bool isConsistentPair(int alpha, int v);
    void setTime(int posx, int v);
};


class LexicographicLT : public Lexicographic {
   public:
    LexicographicLT(Problem &p, std::string n, vec<Variable *> &X, vec<Variable *> &Y) : Lexicographic(p, n, X, Y, true) { }
};

class LexicographicLE : public Lexicographic {
   public:
    LexicographicLE(Problem &p, std::string n, vec<Variable *> &X, vec<Variable *> &Y) : Lexicographic(p, n, X, Y, false) { }
};
}   // namespace Cosoco

#endif /* LEXICOGRAPHIC_H */
