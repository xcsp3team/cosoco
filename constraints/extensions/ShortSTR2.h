#ifndef SHORTSTR2_H
#define SHORTSTR2_H
#include "Extension.h"
#include "mtl/SparseSet.h"
#include "mtl/SparseSetMultiLevel.h"
#include "solver/observers/ObserverDecision.h"

namespace Cosoco {
class ShortSTR2 : public Extension, ObserverDeleteDecision {
   protected:
    SparseSet           Sval;
    SparseSet           Ssup;
    SparseSetMultiLevel validTuples;
    vec<SparseSet *>    gacIdValues;
    vec<int>            lastSize;

    bool isValidTuple(int *tuple);
    void delTuple(int position, int lvl);

   public:
    // Constructors
    ShortSTR2(Problem &p, std::string n, vec<Variable *> &vars, size_t max_n_tuples);
    ShortSTR2(Problem &p, std::string n, vec<Variable *> &vars, Matrix<int> *tuplesFromOtherConstraint);

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

#endif /* SHORTSTTR2_H */
