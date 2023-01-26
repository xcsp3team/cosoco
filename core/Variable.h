#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>

#include "domain/Domain.h"
#include "mtl/Vec.h"

namespace Cosoco {
class Problem;

class Constraint;

class Variable {
   protected:
    bool          addToTrail;   // Need to add in the trail
    unsigned long timestamp;    // Last time the variable was put in queue

   public:
    friend class Solver;

    friend class Problem;

    Problem          &problem;       // The associated problem
    const int         idx;           // The position in problem.variables
    const int         array;         // the array in which the variable appear (-1 if none)
    const std::string _name;         // I do not like _ but... useful for name() function
    Domain           &domain;        // The domain
    vec<Constraint *> constraints;   // Set of constraints where it occurs

    // Datas used for some algorithms (some heuristics and so on...)
    double wdeg;   // The wdeg value


    // Fake data used in some algorithms. Use must be circumscribed to one call of the algorithm
    // See for example Constraint/scopeIsOk
    int fake;

    bool useless;   // Variable with degree0: useless for the search

    // Construction methods
    void delayedConstruction(int id, int nbVars);   // This function is called at the end of the construction of the problem
    void addConstraint(Constraint *c);              // This constraint contains the variable

    Variable(Problem &p, std::string n, Domain &d, int id, int a);   // Do not use it directly (use Problem::createVariable)

    // Delete Methods
    bool delVal(int v, int lvl);     // Do not use directly, use solver's one
    bool delIdv(int idv, int lvl);   // Do not use directly, use solver's one

    // Assign method
    void assignToIdv(int idv, int lvl);   // Do not use directly, use solver's one
    void assignToVal(int v, int lvl);     // Do not use directly, use solver's one

   public:
    // Minor methods
    void display(bool allDetails = false);   // display


    inline const char *name() { return _name.c_str(); }   // The name


    // ----------  shortcut methods
    inline int size() { return domain.size(); }   // The current number of elements in the domain


    inline bool isAssigned() { return domain.size() == 1; }   // is it assign to a value


    // The value of the variable
    inline int value() {
        assert(domain.size() == 1);
        return domain.valueAtPosition(0);
    }


    // The id of the value of the variable
    inline int valueId() {
        assert(domain.size() == 1);
        return domain[0];
    }


    inline bool containsValue(int v) { return domain.containsValue(v); }


    inline bool containsIdv(int idv) { return domain.containsIdv(idv); }


    inline bool isUniqueValue(int v) { return domain.isUniqueValue(v); }


    inline int minimum() { return domain.minimum(); }


    inline int maximum() { return domain.maximum(); }

    static bool haveSameDomainType(vec<Variable *> &vars);
};

}   // namespace Cosoco

#endif /* VARIABLE_H */
