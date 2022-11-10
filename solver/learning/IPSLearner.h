#ifndef COSOCO_IPSLEARNER_H
#define COSOCO_IPSLEARNER_H


#include <core/Problem.h>
#include <solver/observers/ObserverConflict.h>
#include <solver/observers/ObserverDomainReduction.h>

#include "mtl/Vec.h"

namespace Cosoco {
Constraint *rootJustification = (Constraint *)0x1;

class IPSLearner : public ObserverDomainReduction, ObserverConflict {
   protected:
    Solver &                solver;
    Problem &               problem;
    vec<vec<Constraint *> > justifications;
    vec<bool>               selectedVariables;
    vec<Variable *>         extracted;
    bool                    eliminateEntailedVariables;
    bool                    eliminateInitialDomainVariables;
    bool                    eliminateDegreeVariables;

   public:
    IPSLearner(Solver &s, Problem &p);


    void notifyDomainReduction(Variable *x, int idv, Solver &s) override;
    void notifyConflict(Constraint *c, int level) override;
    void notifyDomainAssignment(Variable *x, int idv, Solver &s) override;


    void extract();   // Extract nogood
    bool canEliminateVariable(Variable *x);
    bool canEliminateDeducedVariable(Variable *x);
    bool canEliminateSingletonVariable(Variable *x);
    bool canEliminateDegree(Variable *x);

    int getNbFreeVariables(Constraint *c);
};
}   // namespace Cosoco


#endif   // COSOCO_IPSLEARNER_H
