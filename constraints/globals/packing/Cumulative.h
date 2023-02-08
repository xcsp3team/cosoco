//
// Created by audemard on 10/06/2020.
//

#ifndef COSOCO_CUMULATIVE_H
#define COSOCO_CUMULATIVE_H


#include <constraints/globals/GlobalConstraint.h>
#include <mtl/SparseSet.h>
#include <mtl/SparseSetMultiLevel.h>

#include "solver/observers/ObserverDecision.h"


namespace Cosoco {

class Slot {
   public:
    int start;
    int end;
    int height;
};

class Cumulative;

class TimeTableReasoner {
   public:
    SparseSetMultiLevel relevantTasks;
    SparseSet           ticks;
    vec<int>            offsets;
    vec<Slot>           slots;
    Cumulative         &cumulative;
    int                 nSlots;

    struct CompareStart {
        bool operator()(Slot t1, Slot t2) { return t1.start < t2.start; }
    };

    struct CompareHeight {
        bool operator()(Slot t1, Slot t2) { return t1.height > t2.height; }
    };

    int mandatoryStart(int i);
    int mandatoryEnd(int i);

   public:
    explicit TimeTableReasoner(Cumulative &c);
    int  buildSlots();
    void updateRelevantTasks();
    bool filter();
};


class Cumulative : public GlobalConstraint, ObserverDeleteDecision {
    friend class TimeTableReasoner;

   protected:
    vec<Variable *>   starts;
    vec<int>          wwidths;
    vec<int>          wheights;
    TimeTableReasoner timetableReasoner;

    int limit;
    int horizon;

    int _horizon(vec<Variable *> &);


   public:
    Cumulative(Problem &p, std::string n, vec<Variable *> &vars, vec<Variable *> &scope, vec<int> &l, vec<int> &h, int);

    // Notifications : restore relevantTasks when backtrack is performed
    void notifyDeleteDecision(Variable *x, int v, Solver &s) override;
    void attachSolver(Solver *s) override;

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;

    void filterHeightVariables(vec<Variable *> &_heights);   // Don't want to use multiple inheritance
    void filterWidthVariables(vec<Variable *> &_lenghts);
    void filterLimitVariable(Variable *x);
    

    virtual int maxWidth(int posx);
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool isCorrectlyDefined() override;
};
}   // namespace Cosoco


#endif   // COSOCO_CUMULATIVE_H
