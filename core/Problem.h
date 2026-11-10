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
    vec<Constraint *>        constraints;      // The set of constraints
    vec<Variable *>          variables;        // The set of variables
    std::vector<std::string> arrayNames;       // The name of the variables array
    vec<vec<Variable *> >    variablesArray;   // The different arrays of Variables
    Solver                  *solver;           // The attached solver (initialized to nullptr)

    int                   nbOriginalVars;        // The number of original vars in the model
    int                   nbExtensionsSharded;   // nb extension constraints shared
    vec<VariableMetadata> variablesMetadata;
    std::map<std::string, Variable *>
         mapping;              // The mapping between the name of the variables and the Variable itself. Useful for parsing
    bool isConstructionDone;   // true if the construction of the problem is done
    bool isUnsatisfiable;      // true if a constraint is always false (see buildConstraintFalse)
    // Constructors and delayed initialisation
    Problem();
    virtual ~Problem() = default;   // To let the possibility to cast
    void delayedConstruction();     // Perform additional initialisation at the end of the parsing
    void attachSolver(Solver *s);   // Attach the solver to the problem


    Variable *createVariable(std::string n, Domain &d, int array = -1);   // Add a new variable
    void      addConstraint(Constraint *c);                               // Add a new constraint


    // Problem Statistics
    int  nbVariables() const;
    int  nbConstraints() const;
    int  nbConstraintsOfSize(int size);
    long nbValues();
    int  minimumArity();
    int  maximumArity();
    int  maximumTuplesInExtension();
    int  minimumTuplesInExtension();
    int  minimumDomainSize();
    int  maximumDomainSize();
    void nbTypeOfConstraints(std::map<std::string, int> &);


    // Minor methods
    void        display(bool allDetails = false);
    bool        checkSolution();
    std::string variableName(const Variable *x);
    bool        isUselessVariable(const Variable *x);
};
}   // namespace Cosoco

#endif /* PROBLEM_H */
