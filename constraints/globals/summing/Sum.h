#ifndef SUM_H
#define SUM_H

#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"
namespace Cosoco {

class Sum : public GlobalConstraint {
   public:
    long      limit;
    vec<long> coefficients;

    Sum(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &coefs, long l)
        : GlobalConstraint(p, n, "Sum", vars), limit(l) {
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
}   // namespace Cosoco


#endif /* SUM_H */
