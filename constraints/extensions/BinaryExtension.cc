#include "BinaryExtension.h"

#include <utility>

#include "Extension.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------------------
// check validity and correct definition
//----------------------------------------------------------


bool BinaryExtension::isSatisfiedBy(vec<int> &tuple) {
    int idvx = x->domain.toIdv(tuple[0]);
    int idvy = y->domain.toIdv(tuple[1]);
    return ((isSupport && matrix[idvx][idvy]) || (!isSupport && matrix[idvx][idvy] == false));
}

//----------------------------------------------------------
// Filtering
//----------------------------------------------------------


bool BinaryExtension::filter(Variable *dummy) {
    if(solver->isAssigned(x) == false /*&& x->size()<maxConflictsx*/) {
        for(int idvx : reverse(x->domain)) {
            if(resx[idvx] != -1 && y->containsIdv(resx[idvx]) == true)
                continue;
            bool found = false;
            for(int idvy : y->domain) {
                if((isSupport && matrix[idvx][idvy] == true) || (!isSupport && matrix[idvx][idvy] == false)) {
                    resx[idvx] = idvy;
                    found      = true;
                    break;
                }
            }
            if(found == false && solver->delIdv(x, idvx) == false)
                return false;
        }
    }
    if(solver->isAssigned(y) == false /*&& y->size() < maxConflictsy */) {
        for(int idvy : reverse(y->domain)) {
            if(resy[idvy] != -1 && x->containsIdv(resy[idvy]) == true)
                continue;
            bool found = false;
            for(int idvx : x->domain) {
                if((isSupport && matrix[idvx][idvy] == true) || (!isSupport && matrix[idvx][idvy] == false)) {
                    resy[idvy] = idvx;
                    found      = true;
                    break;
                }
            }
            if(found == false && solver->delIdv(y, idvy) == false)
                return false;
        }
    }
    return true;
}


//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------

BinaryExtension::BinaryExtension(Problem &p, std::string n, bool support, Variable *xx, Variable *yy)
    : Extension(p, n, createScopeVec(xx, yy), 0, support),
      x(xx),
      y(yy),
      maxConflictsx(x->size() + 1),
      maxConflictsy(y->size() + 1),
      nbtuples(0) {
    matrix = new bool *[x->domain.maxSize()];
    for(int i = 0; i < x->domain.maxSize(); i++) {
        matrix[i] = new bool[y->domain.maxSize()];
        std::fill_n(matrix[i], y->domain.maxSize(), false);
    }
}


BinaryExtension::BinaryExtension(Problem &p, std::string n, bool support, Variable *xx, Variable *yy,
                                 BinaryExtension *hasSameTuples)
    : Extension(p, n, createScopeVec(xx, yy), 0, support),
      x(xx),
      y(yy),
      maxConflictsx(x->size() + 1),
      maxConflictsy(y->size() + 1) {
    matrix   = hasSameTuples->matrix;
    nbtuples = hasSameTuples->nbtuples;
}


void BinaryExtension::addTuple(vec<int> &tupleIdv) {
    // Check if tuples are inside domains
    if((tupleIdv[0] != STAR && tupleIdv[0] > x->domain.maxSize()) || tupleIdv[0] < 0)
        return;
    if((tupleIdv[1] != STAR && tupleIdv[1] > y->domain.maxSize()) || tupleIdv[1] < 0)
        return;
    addTuple(tupleIdv[0], tupleIdv[1]);
}


void BinaryExtension::addTuple(int idv1, int idv2) {
    assert(idv1 >= 0 && (idv1 == STAR || idv1 < x->domain.maxSize()));
    assert(idv2 >= 0 && (idv2 == STAR || idv2 < y->domain.maxSize()));
    if(idv1 == STAR) {
        for(int i = 0; i < x->domain.maxSize(); i++) matrix[i][idv2] = true;
        return;
    }
    if(idv2 == STAR) {
        for(int i = 0; i < y->domain.maxSize(); i++) matrix[idv1][i] = true;
        return;
    }

    matrix[idv1][idv2] = true;
    nbtuples++;
}


void BinaryExtension::delayedConstruction(int id) {
    Constraint::delayedConstruction(id);
    resx.growTo(x->domain.maxSize(), -1);
    resy.growTo(y->domain.maxSize(), -1);
}


int BinaryExtension::nbTuples() { return nbtuples; }


struct CSPPropagation {   // x equal idv ??
    Variable *x;
    int       idv;
    bool      equal;
};
