#ifndef MINIMUMCONSTANTGE_H
#define MINIMUMCONSTANTGE_H

#include "MinimumConstant.h"
#include "mtl/Vec.h"

namespace Cosoco {
class MinimumConstantGE : public MinimumConstant {
   protected:
    bool done;   // TODO transform and use entail...
   public:
    MinimumConstantGE(Problem &p, std::string n, vec<Variable *> &vars, int kk) : MinimumConstant(p, n, vars, kk), done(false) { }

    virtual State status() override;
    virtual void  reinitialize() override;

    virtual bool filter(Variable *dummy) override;
    virtual bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco


#endif /* MAXIMUMCONSTANTGE_H */
