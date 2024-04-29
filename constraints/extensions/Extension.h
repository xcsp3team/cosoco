/*
 * File:   Extension.h
 * Author: audemard
 *
 * Created on 24 mars 2015, 11:59
 */

#ifndef EXTENSION_H
#define EXTENSION_H


#include "Matrix.h"
#include "XCSP3Constants.h"
#include "constraints/Constraint.h"

namespace Cosoco {
class Extension : public Constraint {
   public:
    bool    isSupport;
    Matrix *tuples;

    Extension(Problem &p, std::string n, vec<Variable *> &vars, size_t max_n_tuples, bool support)
        : Constraint(p, n, vars), isSupport(support) {
        type   = "Extension";
        tuples = new Matrix(max_n_tuples, vars.size());
    }


    Extension(Problem &p, std::string n, vec<Variable *> &vars, bool support, Matrix *tuplesFromOtherConstraint)
        : Constraint(p, n, vars), isSupport(support), tuples(tuplesFromOtherConstraint) {
        type = "Extension";
    }


    Extension(Problem &p, std::string n, bool support, Variable *xx, Variable *yy)
        : Constraint(p, n, xx, yy), isSupport(support) {
        type = "Extension";
    }


    virtual void addTuple(vec<int> &tupleIdv) {
        // Check if tuples are inside domains
        for(int i = 0; i < tupleIdv.size(); i++)
            if(tupleIdv[i] < 0 || (tupleIdv[i] > scope[i]->domain.maxSize() && tupleIdv[i] != STAR))
                return;
        tuples->addTuple(tupleIdv);
    }


    virtual int nbTuples() { return tuples->nrows(); }
};
}   // namespace Cosoco


#endif /* EXTENSION_H */
