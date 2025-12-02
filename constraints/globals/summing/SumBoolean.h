//
// Created by audemard on 04/10/2025.
//

#ifndef COSOCO_SUMBOOLEAN_H
#define COSOCO_SUMBOOLEAN_H


#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"
#include "xEqOryk.h"
namespace Cosoco {

class SumBoolean : public GlobalConstraint {
   public:
    long limit;

    SumBoolean(Problem &p, std::string n, vec<Variable *> &vars, long l);
    bool        isCorrectlyDefined() override;
    static long sum(vec<int> &tuple);
};


class SumBooleanEQ : public SumBoolean {
   public:
    SumBooleanEQ(Problem &p, std::string n, vec<Variable *> &vars, long l);
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool filter(Variable *x) override;
};

class SumBooleanLE : public SumBoolean, public ObjectiveConstraint {
   public:
    SumBooleanLE(Problem &p, std::string n, vec<Variable *> &vars, long l);
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool filter(Variable *x) override;

    // Functions related to Objective constraint
    void updateBound(long bound) override;
    long maxUpperBound() override;
    long minLowerBound() override;
    long computeScore(vec<int> &solution) override;
};

class SumBooleanGE : public SumBoolean, public ObjectiveConstraint {
   public:
    SumBooleanGE(Problem &p, std::string n, vec<Variable *> &vars, long l);
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool filter(Variable *x) override;

    // Functions related to Objective constraint
    void updateBound(long bound) override;
    long maxUpperBound() override;
    long minLowerBound() override;
    long computeScore(vec<int> &solution) override;
};


class SumBooleanNodes : public GlobalConstraint {
   public:
    long             limit;
    vec<BasicNode *> nodes;
    SumBooleanNodes(Problem &p, std::string n, vec<Variable *> &vars, vec<BasicNode *> &_nodes, long l);
    bool        isCorrectlyDefined() override;
    static long sum(vec<int> &tuple);
};

class SumBooleanNodesEQ : public SumBooleanNodes {
   public:
    SumBooleanNodesEQ(Problem &p, std::string n, vec<Variable *> &vars, vec<BasicNode *> &_nodes, long l);
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool filter(Variable *x) override;
};

class SumBooleanNodesLE : public SumBooleanNodes, public ObjectiveConstraint {
   public:
    SumBooleanNodesLE(Problem &p, std::string n, vec<Variable *> &vars, vec<BasicNode *> &_nodes, long l);
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool filter(Variable *x) override;

    // Functions related to Objective constraint
    void updateBound(long bound) override;
    long maxUpperBound() override;
    long minLowerBound() override;
    long computeScore(vec<int> &solution) override;
};

class SumBooleanNodesGE : public SumBooleanNodes /*, public ObjectiveConstraint */ {
   public:
    SumBooleanNodesGE(Problem &p, std::string n, vec<Variable *> &vars, vec<BasicNode *> &_nodes, long l);
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool filter(Variable *x) override;

    // Functions related to Objective constraint
    // void updateBound(long bound) override;
    // long maxUpperBound() override;
    // long minLowerBound() override;
    // long computeScore(vec<int> &solution) override;
};


}   // namespace Cosoco


#endif   // COSOCO_SUMBOOLEAN_H
