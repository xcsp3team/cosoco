#ifndef COSOCO_CARDINALITYWEAK_H
#define COSOCO_CARDINALITYWEAK_H
#include "constraints/globals/GlobalConstraint.h"
namespace Cosoco {
struct CardData {
    int occs;
    int fixed;
};
class CardinalityWeak : public GlobalConstraint {
   protected:
    std::map<int, CardData> data;

   public:
    CardinalityWeak(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &v, vec<int> &o);
    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;
    void clear();
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool isCorrectlyDefined() override;
};
}   // namespace Cosoco
#endif   // COSOCO_CARDINALITYWEAK_H