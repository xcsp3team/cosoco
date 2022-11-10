// The SAT solver and CSP solver collaborate
// Each time the CSP solver finishes its propagation step, it informs
// the sat solver to propagate all the new facts
// The sat solver can returns a backtrack level or provides a list of propagations facts.
// The process is repaeted until fix point (see the CSP solver)

// Invariant: the both decisions levels are the same.


// Main point: we need to fill the clause database :(
// Nogood from restarts ? yes
// add some conflicts due to sampling ?

#ifndef COSOCO_SATWRAPPER_H
#define COSOCO_SATWRAPPER_H

#include <core/Variable.h>
#include <mtl/Vec.h>
#include <satwrapper/glucose/core/SolverTypes.h>
#include <solver/observers/ObserverDecision.h>
#include <solver/observers/ObserverDomainReduction.h>

#include "./glucose/core/Solver.h"
#define SATISOK -1
namespace Cosoco {

typedef struct CSPPropagation CSPPropagation;
struct CSPPropagation {   // x equal idv ??
    Variable *x;
    int       idv;
    bool      equal;
};


class SatWrapper : public ObserverNewDecision, ObserverDeleteDecision, ObserverDomainReduction {
   protected:
    Solver *cspSolver;

    vec<int>            firstSATVariable;   // firstSATVariable[x->idx] is the first....
    vec<Variable *>     satVar2cspVar;
    vec<Glucose::Lit>   toPropagateInSAT;   // Literals that need to be propagated
    vec<CSPPropagation> toPropagateInCSP;   // props to achieve in CSP solver
    vec<bool>           cspVariableIsBoolean;
    vec<int>            cspLevel;
    Glucose::CRef       crefLearntClause;

   public:
    // ---------------------------------------------------------
    // Construction and init
    // ---------------------------------------------------------
    static Constraint *fake;
    bool               propagateAssertive;

    SatWrapper(Solver *csp);
    Verbose verbose;
    void    createSATEntriesFor(Variable *x);
    // ---------------------------------------------------------
    // CSP <-> SAT variable mapping
    // ---------------------------------------------------------
    Glucose::Solver *          satSolver;
    Glucose::vec<Glucose::Lit> learntClause;

   protected:
    Glucose::Lit X_eq_idv__ToLiteral(Variable *x, int idv);   // X=idv
    Glucose::Lit X_ne_idv__ToLiteral(Variable *x, int idv);   // X!=idv
    void         lit2csp(Glucose::Lit l, CSPPropagation &cspProp);


   public:
    // ---------------------------------------------------------
    // Add propagations to SAT solver
    // ---------------------------------------------------------

    enum SatPropState { USELESS, CONFLICT, PROPAGATE };

    void startCSPPropagatations();                                           // The CSP solver starts its propagation process
    void notifyDomainReduction(Variable *x, int idv, Solver &s) override;    // It informs the SAT solver to a new one  x!=idv
    void notifyDomainAssignment(Variable *x, int idv, Solver &s) override;   // It informs the SAT solver to a new one  x=idv


    int doSATPropagations();                     // At the end the SAT solver has to perform all these propagations
                                                 // it returns the level of backtrack or -1 (if everything is ok)
    SatPropState addAssertiveLiteralInQueue();   // After SAT conflict+backtrack, one needs to propagate the assertive literal

    bool providesNewSATPropagations();   // it provides the new propagations to the CSP solver

    void addNewLevel(Glucose::Lit l);   // We need a new level for this propagated literal
    // ---------------------------------------------------------
    // Add new clauses
    // ---------------------------------------------------------

    void addClause(vec<CSPPropagation> &nogood, bool learnt);

    // ---------------------------------------------------------
    // Minor methods and listener
    // ---------------------------------------------------------

    void         notifyNewDecision(Variable *x, Solver &s) override;
    virtual void notifyDeleteDecision(Variable *x, int v, Solver &s) override;
    void fullBacktrack();   // needed because one has to backtrack before adding clauses (and nogood from restarts need the stack)
};
}   // namespace Cosoco

#endif   // COSOCO_SATWRAPPER_H
