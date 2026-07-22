#ifndef SUM_H
#define SUM_H

#include "InnerOuter.h"
#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"
namespace Cosoco {

class Sum : public GlobalConstraint {
   protected:
    long         min, max;
    long         maxGap;
    virtual void computeBounds() = 0;

   public:
    long limit;
    Sum(Problem &p, std::string n, vec<Variable *> &vars, long l) : GlobalConstraint(p, n, "Sum", vars), limit(l) {
        isPostponable = true;
    }
};

class SimpleSum : public Sum {
   protected:
    void computeBounds() override;

   public:
    SimpleSum(Problem &p, std::string n, vec<Variable *> &vars, long l) : Sum(p, std::move(n), vars, l) {
        maxGap = -1;
        for(Variable *x : vars) {
            int tmp = x->maximum() - x->minimum();
            if(tmp > maxGap)
                maxGap = tmp;
        }
    }

    static long sum(vec<int> &tuple);
};


class WeightedSum : public Sum {
   protected:
    void computeBounds() override;

   public:
    vec<long> coefficients;

    WeightedSum(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &coefs, long l) : Sum(p, std::move(n), vars, l) {
        for(int c : coefs) coefficients.push(c);
        maxGap = -1;
        for(int posx = 0; posx < scope.size(); posx++) {
            Variable *x   = scope[posx];
            int       tmp = std::abs(x->maximum() - x->minimum() * coefficients[posx]);
            if(tmp > maxGap)
                maxGap = tmp;
        }
    }
    bool isCorrectlyDefined() override;   // Implementation inside SumEQ

    long weightedSum(vec<int> &tuple);

    bool delWRTOrder(Variable *x, long l, int coeff, XCSP3Core::OrderType order);
};
}   // namespace Cosoco


#endif /* SUM_H */
