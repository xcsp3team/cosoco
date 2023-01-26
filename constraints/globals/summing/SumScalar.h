#ifndef COSOCO_SUMSCALAR_H
#define COSOCO_SUMSCALAR_H


#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"
#include "solver/Solver.h"
namespace Cosoco {

class SumScalar : public GlobalConstraint {
   protected:
    SparseSet set01vs1;
    int       min, max;

   public:
    int half;

    SumScalar(Problem &p, std::string n, vec<Variable *> &scp)
        : GlobalConstraint(p, n, "Sum Scalar", scp), set01vs1(scp.size() / 2) { }


    bool isCorrectlyDefined() override {
        for(int i = 0; i < half * 2; i++) {
            Variable *x = scope[i];
            if(x->domain.maxSize() != 2 || x->minimum() != 0 || x->maximum() != 1)
                throw std::logic_error("Constraint " + std::to_string(idc) + ": Sum Scalar, vars must be boolean");
        }
        return true;
    }


    long sum(vec<int> &tuple) {
        long sum = 0;
        for(int i = 0; i < half; i++) sum += tuple[i] * tuple[half + i];
        return sum;
    }

    void recomputeBounds() {
        min = max = 0;
        set01vs1.clear();
        for(int i = 0; i < half; i++) {
            Variable *x1 = scope[i];
            Variable *x2 = scope[i + half];

            if(x1->containsValue(1) && x2->containsValue(1)) {
                // if one 1 is missing nothing to do because the product is necessarily 0
                max++;
                if(!x1->containsValue(0) && !x2->containsValue(0))
                    min++;
                else if(x1->size() == 1 || x2->size() == 1)
                    set01vs1.add(i);   // we add i iff we have (0,1) versus 1 (or equivalently 1 versus (0,1)) ; the only way to
                                       // filter here
            }
        }
    }

    void removeFrom01vs1(int value) {
        for(int j : set01vs1) {
            assert((scope[j]->size() == 2 && scope[j + half]->value() == 1) ||
                   (scope[half + j]->size() == 2 && scope[j]->value() == 1));
            if(scope[j]->size() == 2)
                solver->delVal(scope[j], value);
            else
                solver->delVal(scope[j + half], value);
        }
    }
};
}   // namespace Cosoco
#endif   // COSOCO_SUMSCALAR_H
