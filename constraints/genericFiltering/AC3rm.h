#include "TupleIteratorWithoutOrder.h"
#include "constraints/Constraint.h"
#include "mtl/Map.h"

#ifndef AdapterAC3RM_H
#define AdapterAC3RM_H


/**
 *  A class adapter to perform AC3rm on each type of constraint
 *  See IJCAI'07 : Lecoutre, Hemery. A Study of Residual Supports in Arc Consistency.
 */


namespace Cosoco {
class AdapterAC3rm : public Constraint, public ObjectiveConstraint {
   public:
    Constraint *constraint;
    AdapterAC3rm(Constraint *c);

    // All methods have to be override!
    // The constraint method have to be called
    // The filtering technique changes to perform AC3rm

    virtual bool isCorrectlyDefined() override;
    virtual void attachSolver(Solver *s) override;

    // Filtering method, return false if a conflict occurs
    virtual bool  filter(Variable *x) override;
    virtual State status() override;
    virtual void  reinitialize() override;


    // Check tuple validity
    virtual bool isSatisfiedBy(vec<int> &tuple) override;


    virtual void updateBound(long bound) override;

    virtual long maxUpperBound() override;

    virtual long minLowerBound() override;

    virtual long computeScore(vec<int> &solution) override;

    virtual void display(bool allDetails) override;

   protected:
    TupleIteratorWithoutOrder tupleIterator;   // Iterator over tuple in a undetermined order
    vec<vec<vec<int> > >      _residues;       // 1D : posx ; 2D : idv =>  obtain the residue tuple for posx/idv

    bool      revise(Variable *x, int posx);
    bool      seekSupport(int posx, int idv);
    bool      isItTimeToStartFiltering();
    vec<int> *residue(int posx, int idv);
    void      storeResidues(vec<int> &tuple);
};

}   // namespace Cosoco


#endif /* PROXYAC3RM_H */
