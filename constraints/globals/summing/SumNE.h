#ifndef SUMNE_H
#define SUMNE_H

#include "Sum.h"

namespace Cosoco {

class SumNE : public Sum {
   protected:
    int watchedPosition1, watchedPosition2;

    int  findNewWatch();
    long computeSumExceptPos(int pox = -1);
    bool filterUniqueNonSingletonVariable(int posx);

   public:
    SumNE(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &coefs, long l);

    virtual bool filter(Variable *x) override;

    // Checking
    virtual bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco

#endif /* SUMNE_H */
