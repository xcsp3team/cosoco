//
// Created by audemard on 25/04/25.
//

#ifndef NOGOOD_H
#define NOGOOD_H
#include <Extension.h>


namespace Cosoco {
class NoGood : public Extension {
   protected:
   public:
    // Constructors
    NoGood(Problem &p, std::string n, vec<Variable *> &vars);
    NoGood(Problem &p, std::string n, vec<Variable *> &vars, Matrix<int> *tuplesFromOtherConstraint);

    // filtering
    bool filter(Variable *x) override;
    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool isCorrectlyDefined() override;
};
}   // namespace Cosoco


#endif   // NOGOOD_H
