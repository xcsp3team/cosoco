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
    long      maxGap;


    Sum(Problem &p, vec<Variable *> &vars, vec<int> &coefs, long l) : GlobalConstraint(p, "Sum", vars), limit(l) {
        for(int c : coefs) coefficients.push(c);
        isPostponable = true;
        maxGap        = LONG_MIN;
        for(int posx = 0; posx < scope.size(); posx++) {
            Variable *x = scope[posx];
            maxGap      = std::max(maxGap, std::abs(coefficients[posx] * (x->maximum() - x->minimum())));
        }
    }
    bool isCorrectlyDefined() override;   // Implementation inside SumEQ

    long weightedSum(vec<int> &tuple) {
        long sum = 0;
        for(int i = 0; i < tuple.size(); i++) sum += coefficients[i] * ((long)tuple[i]);
        return sum;
    }
};

class SumIncremental : public Sum {
   protected:
    long min, max, fixed;
    void computeBounds();

   public:
    SumIncremental(Problem &p, vec<Variable *> &vars, vec<int> &coefs, long l) : Sum(p, vars, coefs, l) {
        min   = 0;
        max   = 0;
        fixed = 0;
    }

    void assignVariable(Variable *x, int posx) override;
    void unassignVariable(Variable *x, int posx, int v) override;
};
}   // namespace Cosoco


#endif /* SUM_H */
