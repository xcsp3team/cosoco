#ifndef SUM_H
#define SUM_H

#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"
namespace Cosoco {

class Sum : public GlobalConstraint {
   protected:
    long         min, max;
    virtual void computeBounds() = 0;

   public:
    long limit;
    Sum(Problem &p, std::string n, vec<Variable *> &vars, long l) : GlobalConstraint(p, n, "Sum", vars), limit(l) { }
};

class SimpleSum : public Sum {
   protected:
    void computeBounds() override;
    int  maxGap;

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
   public:
    vec<long> coefficients;

    WeightedSum(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &coefs, long l) : Sum(p, std::move(n), vars, l) {
        for(int c : coefs) coefficients.push(c);
        isPostponable = true;
    }
    bool isCorrectlyDefined() override;   // Implementation inside SumEQ

    long weightedSum(vec<int> &tuple);
    void computeBounds() override;
};
}   // namespace Cosoco


#endif /* SUM_H */
