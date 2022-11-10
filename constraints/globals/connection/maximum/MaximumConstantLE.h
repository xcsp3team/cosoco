#ifndef MAXIMUMCONSTANTLE_H
#define MAXIMUMCONSTANTLE_H

#include "MaximumConstant.h"
#include "mtl/Vec.h"
namespace Cosoco {
class MaximumConstantLE : public MaximumConstant {
    bool done;   // TODO transform and use entail...
   public:
    MaximumConstantLE(Problem &p, std::string n, vec<Variable *> &vars, int kk) : MaximumConstant(p, n, vars, kk), done(false) {
        type = "Maximum Constant LE";
    }

    State status() override;

    void reinitialize() override;

    bool filter(Variable *dummy) override;
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif /* MAXIMUMCONSTANTLE_H */
