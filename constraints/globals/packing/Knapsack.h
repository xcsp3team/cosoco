//
// Created by audemard on 27/03/2026.
//

#ifndef COSOCO_KNAPSACK_H
#define COSOCO_KNAPSACK_H
#include "GlobalConstraint.h"

namespace Cosoco {
class Knapsack : public GlobalConstraint {
   protected:
    vec<Variable *> vars;
    vec<int>        weights;
    vec<int>        profits;

    int  wlimit;
    int  plimit;
    long wmin, wmax;
    long pmin, pmax;
    int  minWeight, maxWeight;
    int  minProfit, maxProfit;
    bool basic;

   public:
    Knapsack(Problem &p, std::string n, vec<Variable *> &_vars, vec<int> &_w, vec<int> &_p, int wl, int pl);
    bool isCorrectlyDefined() override;


    // Filtering method, return false if a conflict occurs
    void recomputeBounds();
    bool filter(Variable *x) override;
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

class KnapsackVARW : public Knapsack {
   protected:
    Variable *varWLimit;

   public:
    KnapsackVARW(Problem &p, std::string n, vec<Variable *> &_vars, vec<int> &_w, vec<int> &_p, Variable *wl, int pl);
    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

class KnapsackVARP : public Knapsack {
   protected:
    Variable *varPLimit;

   public:
    KnapsackVARP(Problem &p, std::string n, vec<Variable *> &_vars, vec<int> &_w, vec<int> &_p, int wl, Variable *pl);
    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};

class KnapsackVARWP : public Knapsack {
   protected:
    Variable *varPLimit;
    Variable *varWLimit;


   public:
    KnapsackVARWP(Problem &p, std::string n, vec<Variable *> &_vars, vec<int> &_w, vec<int> &_p, Variable *wl, Variable *pl);
    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
};
}   // namespace Cosoco
#endif   // COSOCO_KNAPSACK_H
