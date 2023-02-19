#ifndef PROBLEM_H
#define PROBLEM_H

#include <map>
#include <string>

#include "constraints/Constraint.h"
#include "core/Variable.h"
#include "mtl/Vec.h"
#include "optimizer/ObjectiveConstraint.h"

namespace Cosoco {


class Solver;


class Problem {
   public:
    const std::string     name;                 // The name of the problem
    Solver               *solver;               // The attached solver (initialized to nullptr)
    vec<Constraint *>     constraints;          // The set of constraints
    vec<Variable *>       variables;            // The set of variables
    vec<vec<Variable *> > variablesArray;       // The different arrays of Variables
    bool                  isConstructionDone;   // true if the construction of the problem is done
    bool                  isBinary;             // Not currently used
    std::map<std::string, Variable *>
        mapping;   // The mapping between the name of the variables and the Variable itself. Useful for parsing

    // Constructors and delayed initialisation
    Problem(std::string n);
    virtual ~Problem() = default;   // To let the possibility to cast
    void delayedConstruction();     // Perform additional initialisation at the end of the parsing
    void attachSolver(Solver *s);   // Attach the solver to the problem


    Variable *createVariable(std::string n, Domain &d, int array = -1);   // Add a new variable
    void      addConstraint(Constraint *c);                               // Add a new constraint


    // Problem Statistics
    int  nbVariables() const;
    int  nbConstraints() const;
    int  nbConstraintsOfSize(int size);
    int  minimumArity();
    int  maximumArity();
    int  maximumTuplesInExtension();
    int  minimumTuplesInExtension();
    int  minimumDomainSize();
    int  maximumDomainSize();
    void nbTypeOfConstraints(std::map<std::string, int> &);


    // Minor methods
    void display(bool allDetails = false);
    bool checkSolution();
};
}   // namespace Cosoco

#endif /* PROBLEM_H */
