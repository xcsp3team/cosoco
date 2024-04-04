/*
 * File:   Extension.h
 * Author: audemard
 *
 * Created on 24 mars 2015, 11:59
 */

#ifndef EXTENSION_H
#define EXTENSION_H


#include "XCSP3Constants.h"
#include "constraints/Constraint.h"

namespace Cosoco {
class Extension : public Constraint {
   public:
    bool            isSupport;
    vec<vec<int> > &tuples;         // BE CAREFUL :  indices of domain values are stored
    vec<vec<int> >  tuplesStored;   // Can be empty if tuples are related to other constraint( see constraint group)


    Extension(Problem &p, std::string n, vec<Variable *> &vars, bool support)
        : Constraint(p, n, vars), isSupport(support), tuples(tuplesStored) {
        type = "Extension";
    }


    Extension(Problem &p, std::string n, vec<Variable *> &vars, bool support, vec<vec<int> > &tuplesFromOtherConstraint)
        : Constraint(p, n, vars), isSupport(support), tuples(tuplesFromOtherConstraint) { }


    Extension(Problem &p, std::string n, bool support, Variable *xx, Variable *yy)
        : Constraint(p, n, xx, yy), isSupport(support), tuples(tuplesStored) {
        type = "Extension";
    }


    virtual void addTuple(vec<int> &tupleIdv) {
        // Check if tuples are inside domains
        for(int i = 0; i < tupleIdv.size(); i++)
            if(tupleIdv[i] < 0 || (tupleIdv[i] > scope[i]->domain.maxSize() && tupleIdv[i] != STAR))
                return;
        tuplesStored.push();
        tupleIdv.copyTo(tuplesStored.last());
    }


    virtual int nbTuples() { return tuples.size(); }
};
}   // namespace Cosoco


#endif /* EXTENSION_H */
