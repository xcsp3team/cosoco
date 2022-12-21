//
// Created by audemard on 10/06/2020.
//

#ifndef COSOCO_CUMULATIVE_H
#define COSOCO_CUMULATIVE_H


#include <constraints/globals/GlobalConstraint.h>
#include <mtl/SparseSet.h>
#include <mtl/SparseSetMultiLevel.h>

namespace Cosoco {

class Slot {
   public:
    int start;

    int end;

    int height;
};


class Cumulative : public GlobalConstraint {
   protected:
    vec<Variable*> variables;
    vec<int> lengths;
    vec<int> heights;

    int limit;
    int horizon;

    SparseSet ticks;
    vec<int> offsets;
    vec<Slot> slots;
    int nSlots;

    SparseSetMultiLevel omega;

    int _horizon(vec<Variable *> &, vec<int> &l);
    int mandatoryStart(int i) { return variables[i]->maximum(); }
    int mandatoryEnd(int i) { return variables[i]->minimum() + lengths[i]; }
    int buildTimeTable();

    struct CompareStart {
        bool operator()(Slot t1, Slot t2) { return t1.start < t2.start; }
    };

    struct CompareHeight {
        bool operator()(Slot t1, Slot t2) { return t1.height > t2.height; }
    };

   public:
    Cumulative(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &, vec<int> &, int, Variable* limitV = nullptr);

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool isCorrectlyDefined() override;
};
}   // namespace Cosoco


#endif   // COSOCO_CUMULATIVE_H
