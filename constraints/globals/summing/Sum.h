#ifndef SUM_H
#define SUM_H

#include "InnerOuter.h"
#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"


namespace Cosoco {

class Sum : public GlobalConstraint {
   protected:
    long min, max, fixed;

   public:
    long limit;
    long maxGap;


    Sum(Problem &p, vec<Variable *> &vars, long l) : GlobalConstraint(p, "Sum", vars), limit(l) {
        isPostponable = true;
        min = max = fixed = 0;
    }
};


class SimpleSum : public Sum {
   protected:
    void computeBounds();

   public:
    SimpleSum(Problem &p, vec<Variable *> &vars, long l) : Sum(p, vars, l) {
        maxGap = -1;
        for(Variable *x : vars) {
            int tmp = x->maximum() - x->minimum();
            if(tmp > maxGap)
                maxGap = tmp;
        }
    }

    static long sum(vec<int> &tuple);
    void        assignVariable(Variable *x, int posx) override;
    void        unassignVariable(Variable *x, int posx, int v) override;
};


class WeightedSum : public Sum {
   protected:
    void computeBounds();

   public:
    vec<long> coefficients;

    WeightedSum(Problem &p, vec<Variable *> &vars, vec<int> &coefs, long l) : Sum(p, vars, l) {
        for(int c : coefs) coefficients.push(c);
        maxGap = -1;
        for(int posx = 0; posx < scope.size(); posx++) {
            Variable *x   = scope[posx];
            int       tmp = std::abs((x->maximum() - x->minimum()) * coefficients[posx]);
            if(tmp > maxGap)
                maxGap = tmp;
        }
    }
    bool isCorrectlyDefined() override;   // Implementation inside SumEQ
    void assignVariable(Variable *x, int posx) override;
    void unassignVariable(Variable *x, int posx, int v) override;


    long weightedSum(vec<int> &tuple);

    bool delWRTOrder(Variable *x, long l, int coeff, XCSP3Core::OrderType order);
};

}   // namespace Cosoco


#endif /* SUM_H */
