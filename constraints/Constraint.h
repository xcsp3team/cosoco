#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <iostream>
#include <string>

#include "XCSP3Tree.h"
#include "core/Problem.h"
#include "core/Variable.h"
#include "mtl/SparseSet.h"
#include "mtl/SparseSetOfVariables.h"
#include "mtl/Vec.h"


#define NOTINSCOPE -1
namespace Cosoco {

enum State { CONSISTENT, INCONSISTENT, UNDEF };

class Problem;

class Variable;

class Solver;

class Constraint {
   protected:
    vec<int> idxToScopePosition;
    bool     indexesAreValues;
    vec<int> current;   // to avoid creation during search
    int      arity;     // arity=scope.size()
   public:
    Problem &       problem;                  // The linked problem
    Solver *        solver;                   // The linked solver
    std::string     name;                     // Name in the problem description
    std::string     type;                     // The type : ext, int, alldiff.... (useful for displaying)
    int             idc;                      // The id in problem.constraints
    vec<Variable *> scope;                    // The scope of the constraint
    SparseSet       unassignedVariablesIdx;   // The id of the variables not assigned in the constraint
    unsigned long   timestamp;                // Last time constraint was checked

    // Special datas used for some algorithms (some heuristics and so on...)
    vec<double> wdeg;   // The wdeg value associated to each var of the constraint


    // Constructors and delayed initialisation
    Constraint(Problem &p, std::string n, vec<Variable *> &vars);
    Constraint(Problem &p, std::string n, Variable *x);                             // For unary constraints
    Constraint(Problem &p, std::string n, Variable *x, Variable *y);                // For binary constraints
    Constraint(Problem &p, std::string n, Variable *x, Variable *y, Variable *z);   // For Ternary constraints
    Constraint(Problem &p, std::string n,
               int nbVars);   // For global constraint when scope is related to many different vars.
                              // Do not forget to call scopeInitialisation and to specify nb vars in scope

    bool         scopeIsOk();            // is the scope is correctly defined?
    virtual bool isCorrectlyDefined();   // is the constraint is correctly defined?
    void         scopeInitialisation(
                vec<Variable *> &vars);         // Usefull  when scope has to be built with different vars (global constraint)
    virtual void delayedConstruction(int id);   // Called at the end of the construction of the problems

    // Filtering method, return false if a conflict occurs
    bool filterFrom(Variable *x);   // the function called when the constraint need to be filtered
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
                vec<int> &      tuple);   // Given an interpretation, extract the related vector associated to the scope
    bool isSatisfiedByOfIndexes(vec<int> &tupleOfIndex);

    // Extract tuples
   public:
    // Display
    virtual void display(bool allDetails = false);
    virtual void attachSolver(Solver *s);
    static void  toExtensionConstraint(XCSP3Core::Tree *tree, vec<Variable *> &scope, std::vector<std::vector<int> > &tuples,
                                       bool &isSupport);   // Extract Extensional . Return nullptr if too many tuples
};

};   // namespace Cosoco

#endif /* CONSTRAINT_H */
