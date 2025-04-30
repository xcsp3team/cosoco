//
// Created by audemard on 25/04/25.
//

#ifndef NOGOOD_H
#define NOGOOD_H
#include <Constraint.h>


namespace Cosoco {
class NoGood : public Constraint {
   protected:
   public:
    vec<int> tuple;

    // Constructors
    NoGood(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &tuple);
    // NoGood(Problem &p, std::string n, vec<Variable *> &vars, vec);

    // filtering
    bool filter(Variable *x) override;
    // checking
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool isCorrectlyDefined() override;
};
}   // namespace Cosoco


#endif   // NOGOOD_H
