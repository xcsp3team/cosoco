//
// Created by audemard on 01/02/2022.
//

#ifndef COSOCO_PRECEDENCECONSTRAINT_H
#define COSOCO_PRECEDENCECONSTRAINT_H

#include "constraints/globals/GlobalConstraint.h"
#include "mtl/Vec.h"
#include "ObserverDecision.h"

namespace Cosoco {

class Precedence : public GlobalConstraint, ObserverDeleteDecision {
    int r, k;


   bool covered;

    /**
	 * The number of values that still must be considered because possibly assigned (those at indexes ranging from 0 to
	 * size-1)
     */
   int size;

   bool reinit = true;

   vec<int> firsts;

   int dist = 5; // if dist is r, do we have GAC?

   vec<int> values;

   public :





    Precedence(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &vs);

     // Filtering method, return false if a conflict occurs
     virtual bool filter(Variable *x) override;
     void notifyDeleteDecision(Variable *x, int v, Solver &s) override;

     // Checking
     virtual bool isSatisfiedBy(vec<int> &tuple) override;
     virtual bool isCorrectlyDefined() override;
};

}

#endif   // COSOCO_PRECEDENCECONSTRAINT_H
