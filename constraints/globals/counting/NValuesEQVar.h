#ifndef COSOCO_NVALUESEQVAR_H
#define COSOCO_NVALUESEQVAR_H

#include <constraints/globals/GlobalConstraint.h>

#include <set>


namespace Cosoco {

class NValuesEQVar : public GlobalConstraint {
   public:
    Variable *k;
    int       szVector;

    std::set<int> fixedValues;
    std::set<int> unfixedVariables;

    NValuesEQVar(Problem &p, std::string n, vec<Variable *> &vars, Variable *x);

    virtual bool filter(Variable *x) override;
    void         initializeSets();

    // Checking
    virtual bool isSatisfiedBy(vec<int> &tuple) override;
    virtual bool isCorrectlyDefined() override;
    int          countDistinct(vec<int> &tuple);
};

}   // namespace Cosoco
#endif   // COSOCO_NVALUESEQVAR_H
