
#ifndef COSOCO_FAKEFORNOGOOD_H
#define COSOCO_FAKEFORNOGOOD_H
#include "constraints/Constraint.h"
namespace Cosoco {
class FakeForNoGood : public Extension {
   public:
    vec<vec<int>>  vectuples;
    FakeForNoGood *same;
    FakeForNoGood(Problem &p, std::string n, vec<Variable *> &scope) : Extension(p, n, scope, 0, false) {
        type = "NoGood";
        same = nullptr;
    }
    FakeForNoGood(Problem &p, std::string n, vec<Variable *> &scope, FakeForNoGood *s) : Extension(p, n, scope, 0, false) {
        type = "NoGood";
        same = s;
    }

    bool filter(Variable *x) override { return true; }
    bool isSatisfiedBy(vec<int> &tuple) override { return true; }
    void addTuple(vec<int> &tupleIdv) override {
        vectuples.push();
        tupleIdv.copyTo(vectuples.last());
    }

    vec<vec<int>> &getTuples() {
        if(same != nullptr)
            return same->getTuples();
        return vectuples;
    }
    virtual size_t nbTuples() { return getTuples().size(); }
};
}   // namespace Cosoco


#endif   // COSOCO_FAKEFORNOGOOD_H
