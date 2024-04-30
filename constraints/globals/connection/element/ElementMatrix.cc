#include "ElementMatrix.h"

#include "solver/Solver.h"


using namespace Cosoco;

//----------------------------------------------------------
// check validity
//----------------------------------------------------------

bool ElementMatrix::isSatisfiedBy(vec<int> &tuple) {
    int i = tuple[rindexPosition], j = tuple[cindexPosition];
    return tuple[i * matrix.size() + j] == value;
}


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------


bool ElementMatrix::filter(Variable *x) {
    // filtering the domain of rindex

    if(solver->decisionLevel() == 0) {
        for(int idv : reverse(rindex->domain)) {
            int v = rindex->domain.toVal(idv);
            if((v < 0 || v > matrix.size()) && solver->delIdv(rindex, idv) == false)
                return false;
        }

        for(int idv : reverse(cindex->domain)) {
            int v = cindex->domain.toVal(idv);
            if((v < 0 || v > matrix[0].size()) && solver->delIdv(cindex, idv) == false)
                return false;
        }
    }

    if(rindex->size() > 1) {
        for(int idv : reverse(rindex->domain)) {
            int rv = rindex->domain.toVal(idv);
            int cv = rsentinels[rv];
            if(cv != -1 && cindex->containsValue(cv) && matrix[rv][cv]->containsValue(value))
                continue;
            bool found = false;
            for(int b : reverse(cindex->domain)) {
                int cv2 = cindex->domain.toVal(b);
                if(matrix[rv][cv2]->containsValue(value)) {
                    found          = true;
                    rsentinels[rv] = cv2;
                    break;
                }
            }
            if(!found && solver->delIdv(rindex, idv) == false)
                return false;
        }
    }

    // filtering the domain of cindex
    if(cindex->size() > 1) {
        for(int idv : reverse(cindex->domain)) {
            int cv = cindex->domain.toVal(idv);
            int rv = csentinels[cv];
            if(rv != -1 && rindex->containsValue(rv) && matrix[rv][cv]->containsValue(value))
                continue;
            bool found = false;
            for(int a : reverse(rindex->domain)) {
                int rv2 = rindex->domain.toVal(a);
                if(matrix[rv2][cv]->containsValue(value)) {
                    found          = true;
                    csentinels[cv] = rv2;
                    break;
                }
            }
            if(!found && solver->delIdv(cindex, idv) == false)
                return false;
        }
    }
    // be careful : not a else because of statements above that may modify the domain of indexes
    if(rindex->size() == 1 && cindex->size() == 1)
        if(solver->assignToVal(matrix[rindex->value()][cindex->value()], value) == false)
            return false;
    return true;
}


//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------

ElementMatrix::ElementMatrix(Problem &p, std::string n, vec<vec<Variable *> > &m, Variable *ri, Variable *ci, int v)
    : GlobalConstraint(p, n, "ElementMatrix", 0) {
    matrix.growTo(m.size());
    vec<Variable *> vars;
    for(int i = 0; i < m.size(); i++) {
        for(int j = 0; j < m[i].size(); j++) {
            matrix[i].push(m[i][j]);
            vars.push(m[i][j]);
        }
    }
    rindex = ri;
    cindex = ci;
    value  = v;

    rindexPosition = vars.firstOccurrenceOf(rindex);
    cindexPosition = vars.firstOccurrenceOf(cindex);

    if(rindexPosition == -1) {
        vars.push(rindex);
        rindexPosition = vars.size() - 1;
    }

    if(cindexPosition == -1) {
        vars.push(rindex);
        cindexPosition = vars.size() - 1;
    }

    addToScope(vars);

    rsentinels.growTo(matrix.size(), -1);
    csentinels.growTo(matrix.size(), -1);
}