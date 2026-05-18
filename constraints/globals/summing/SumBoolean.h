//
// Created by audemard on 04/10/2025.
//

#ifndef COSOCO_SUMBOOLEAN_H
#define COSOCO_SUMBOOLEAN_H


#include "BasicNodes.h"
#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"
namespace Cosoco {

class SumBoolean : public GlobalConstraint, public ObjectiveConstraint {
   public:
    long limit;

    SumBoolean(Problem &p, std::string n, vec<Variable *> &vars, long l);
    bool        isCorrectlyDefined() override;
    static long sum(vec<int> &tuple);

    // Functions related to Objective constraint
    void updateBound(long bound) override;
    long maxUpperBound() override;
    long minLowerBound() override;
    long computeScore(vec<int> &solution) override;
};


class SumBooleanEQ : public SumBoolean {
   public:
    SumBooleanEQ(Problem &p, std::string n, vec<Variable *> &vars, long l);
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool filter(Variable *x) override;
};

class SumBooleanLE : public SumBoolean {
   public:
    SumBooleanLE(Problem &p, std::string n, vec<Variable *> &vars, long l);
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool filter(Variable *x) override;
};

class SumBooleanGE : public SumBoolean {
   public:
    SumBooleanGE(Problem &p, std::string n, vec<Variable *> &vars, long l);
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool filter(Variable *x) override;
};

class SumBooleanGen : public GlobalConstraint, public ObjectiveConstraint {
   public:
    long             limit;
    vec<BasicNode *> nodes;

    SumBooleanGen(Problem &p, std::string n, vec<BasicNode *> &nodes, vec<Variable *> &vars, long l);
    bool isCorrectlyDefined() override;

    // Functions related to Objective constraint
    void updateBound(long bound) override;
    long maxUpperBound() override;
    long minLowerBound() override;
    long computeScore(vec<int> &solution) override;
};

class SumBooleanGenLE : public SumBooleanGen {
   public:
    SumBooleanGenLE(Problem &p, std::string n, vec<BasicNode *> &nodes, vec<Variable *> &vars, long l);
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool filter(Variable *x) override;
};

class SumBooleanGenGE : public SumBooleanGen {
   public:
    SumBooleanGenGE(Problem &p, std::string n, vec<BasicNode *> &nodes, vec<Variable *> &vars, long l);
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool filter(Variable *x) override;
};

class SumBooleanGenEQ : public SumBooleanGen {
   public:
    SumBooleanGenEQ(Problem &p, std::string n, vec<BasicNode *> &nodes, vec<Variable *> &vars, long l);
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool filter(Variable *x) override;
};

}   // namespace Cosoco


#endif   // COSOCO_SUMBOOLEAN_H
