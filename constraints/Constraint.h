#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <iostream>
#include <string>

#include "core/Problem.h"
#include "core/Variable.h"
#include "mtl/SparseSet.h"
#include "mtl/SparseSetOfVariables.h"
#include "mtl/Vec.h"


#define NOTINSCOPE -1

#ifdef USE_XCSP3
#include <vector>
namespace XCSP3Core {
class Tree;
}
#endif

namespace Cosoco {

enum State { CONSISTENT, INCONSISTENT, UNDEF };

class Problem;

class Variable;

class Solver;
typedef vec<Variable *> vecVariables;

class Constraint {
   protected:
    vec<int> idxToScopePosition;
    bool     indexesAreValues;
    vec<int> current;   // to avoid creation during search
    int      arity;     // arity=scope.size()
   public:
    Problem        &problem;                  // The linked problem
    Solver         *solver;                   // The linked solver
    std::string     name;                     // Name in the problem description
    std::string     type;                     // The type : ext, int, alldiff.... (useful for displaying)
    int             idc;                      // The id in problem.constraints
    vec<Variable *> scope;                    // The scope of the constraint
    SparseSet       unassignedVariablesIdx;   // The id of the variables not assigned in the constraint
    unsigned long   timestamp;                // Last time constraint was checked
    bool            isPostponable = false;    // If the tag is trus the constraint is filtered after the fixed point
    Variable       *postponedBy;              // The variable wanted to make the filtering
    bool            isDisabled = false;       // The constraint is enabled or not (useful an optimisation solving)
    // Special datas used for some algorithms (some heuristics and so on...)
    vec<double> wdeg;   // The wdeg value associated to each var of the constraint


    // Constructors and delayed initialisation
    Constraint(Problem &p, std::string n, vec<Variable *> &vars);
    Constraint(Problem &p, std::string n);   // For global constraints, the scope is managed by themselves
    bool         scopeIsOk();                // is the scope is correctly defined?
    virtual bool isCorrectlyDefined();       // is the constraint is correctly defined?
    void         addToScope(vec<Variable *> &vars);
    void         addToScope(Variable *x);

    virtual void delayedConstruction(int id);   // Called at the end of the construction of the problems
    void         makeDelayedConstruction(int id);
    // Filtering method, return false if a conflict occurs
    bool filterFrom(Variable *x);   // the function called when the constraint need to be filtered
    bool postpone();

   protected:
    virtual bool filter(Variable *x) = 0;   // Called by all constraints. protected because call is done by filterFrom

   public:
    virtual State status();         // Override this method if it is known when CONSISTENT / INCONSISTENT
    virtual void  reinitialize();   // The constraint must be reinitialized (after a full backtrak)

    // Assign and unassign variables
    void assignVariable(Variable *x);
    void unassignVariable(Variable *x);

    // map idx variable to scope position
    int toScopePosition(Variable *x);
    int toScopePosition(int idx);


    // Check tuple validity
    virtual bool isSatisfiedBy(vec<int> &tuple) = 0;
    void         extractConstraintTupleFromInterpretation(
                const vec<int> &interpretation,
                vec<int>       &tuple);   // Given an interpretation, extract the related vector associated to the scope
    bool isSatisfiedByOfIndexes(vec<int> &tupleOfIndex);

    // Extract tuples
   public:
    // Display
    virtual void display(bool allDetails = false);
    virtual void attachSolver(Solver *s);
#ifdef USE_XCSP3
    static void toExtensionConstraint(XCSP3Core::Tree *tree, vec<Variable *> &scope, std::vector<std::vector<int> > &tuples,
                                      bool &isSupport);   // Extract Extensional . Return nullptr if too many tuples
#endif

   protected:
    // All this part simplify scope initialisation
    static vec<Variable *> temporary;

    static void dosScopeInitialisation(Variable *v) {
        if(temporary.contains(v))
            return;
        temporary.push(v);
    }

    static void dosScopeInitialisation(vecVariables *vars) {
        for(Variable *v : *vars) dosScopeInitialisation(v);
    }
    static void initScope() { }

    template <typename T, typename... Args>
    static void initScope(T &t, Args... args) {
        dosScopeInitialisation(t);
        initScope(args...);
    }

    template <typename... Args>
    static vecVariables &createScopeVec(Args &&...args) {
        temporary.clear();
        initScope(args...);
        return temporary;
    }
};


};   // namespace Cosoco

#endif /* CONSTRAINT_H */
