#ifndef SUM_H
#define SUM_H

#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"
#include "solver/observers/ObserverDecision.h"
#include "solver/observers/ObserverDomainReduction.h"
namespace Cosoco {

class Sum : public GlobalConstraint {
   public:
    long      limit;
    vec<long> coefficients;

    Sum(Problem &p, vec<Variable *> &vars, vec<int> &coefs, long l) : GlobalConstraint(p, "Sum", vars), limit(l) {
        for(int c : coefs) coefficients.push(c);
        isPostponable = true;
    }
    bool isCorrectlyDefined() override;   // Implementation inside SumEQ

    long weightedSum(vec<int> &tuple) {
        long sum = 0;
        for(int i = 0; i < tuple.size(); i++) sum += coefficients[i] * ((long)tuple[i]);
        return sum;
    }
};

class SumIncremental : public Sum, public ObserverDeleteDecision, public ObserverDomainReduction {
   protected:
    long min, max;
    void computeBounds();

   public:
    SumIncremental(Problem &p, vec<Variable *> &vars, vec<int> &coefs, long l) : Sum(p, vars, coefs, l) {
        min = 0;
        max = 0;
    }

    void notifyDeleteDecision(Variable *x, int v, Solver &s, bool isFull) override;
    void notifyDomainAssignment(Variable *x, int idv, Solver &s) override;
    void notifyDomainReduction(Variable *x, int idv, Solver &s) override { }
    void delayedConstruction(int id) override;
};
}   // namespace Cosoco


#endif /* SUM_H */
