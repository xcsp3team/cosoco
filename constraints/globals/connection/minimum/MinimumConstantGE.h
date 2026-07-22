#ifndef MINIMUMCONSTANTGE_H
#define MINIMUMCONSTANTGE_H

#include "MinimumConstant.h"
#include "mtl/Vec.h"

namespace Cosoco {
class MinimumConstantGE : public MinimumConstant {
   protected:
    bool done;   // TODO transform and use entail...
   public:
    MinimumConstantGE(Problem &p, vec<Variable *> &vars, int kk) : MinimumConstant(p, vars, kk), done(false) { }

    State status() override;
    void  reinitialize() override;

    bool filter(Variable *dummy) override;
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif /* MAXIMUMCONSTANTGE_H */
