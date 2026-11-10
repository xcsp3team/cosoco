#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>

#include "domain/Domain.h"
#include "mtl/Vec.h"

namespace Cosoco {
class Problem;

class Constraint;

struct VariableMetadata {
    const int         array;
    const std::string name;      // I do not like _ but... useful for name() function
    bool              useless;   // Variable with degree0: useless for the search
};

class Variable {
   protected:
    unsigned long timestamp;   // Last time the variable was put in queue

   public:
    friend class Solver;

    friend class Problem;

    const int                         idx;           // The position in problem.variables
    Domain                           &domain;        // The domain
    double                            wdeg;          // The wdeg value
    vec<std::pair<Constraint *, int>> constraints;   // Set of constraints where it occurs with the position in the scope

    // Construction methods
    void delayedConstruction(int id, int nbVars);   // This function is called at the end of the construction of the problem
    void addConstraint(Constraint *c, int posx);    // This constraint contains the variable at position posx

    Variable(Problem &p, std::string n, Domain &d, int id, int a);   // Do not use it directly (use Problem::createVariable)

    // Delete Methods
    void delIdv(int idv, int lvl);      // Do not use directly, use solver's one. Returns true if it has to be added to the trail
    void delValuesGE(int v, int lvl);   // Do not use directly, use solver's one. Returns true if it has to be added to the trail
    void delValuesLE(int v, int lvl);   // Do not use directly, use solver's one. Returns true if it has to be added to the trail


    // Assign method
    void assignToIdv(int idv, int lvl);   // Don't use directly, use solver's one. Returns true if it has to be added to the trail
    void assignToVal(int v, int lvl);     // Don't use directly, use solver's one. Returns true if it has to be added to the trail

    // Restore State of the variable after backtrack
    bool isRecorded(int level) { return domain.idvs.lastRemovedLevel() == level; }
    void restore(int level) const { domain.restore(level); }

    // Minor methods
    void display(bool allDetails = false);   // displayCurrentBranch


    // ----------  shortcut methods
    int size() { return domain.size(); }   // The current number of elements in the domain


    bool isAssigned() { return domain.size() == 1; }   // is it assign to a value


    // The value of the variable
    int value() {
        assert(domain.size() == 1);
        return domain.valueAtPosition(0);
    }


    // The id of the value of the variable
    int valueId() {
        assert(domain.size() == 1);
        return domain[0];
    }


    bool containsValue(int v) { return domain.containsValue(v); }


    bool containsIdv(int idv) { return domain.containsIdv(idv); }


    bool isUniqueValue(int v) { return domain.isUniqueValue(v); }


    int minimum() { return domain.minimum(); }


    int maximum() { return domain.maximum(); }

    static bool haveSameDomainType(vec<Variable *> &vars);

    bool isBoolean() { return domain.isBoolean(); }

    static int maxDomainSize(vec<Variable *> &list);

    static int sumDomainSize(vec<Variable *> &list);
};

}   // namespace Cosoco

#endif /* VARIABLE_H */
