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
    return (*matrix)[idvx][idvy] == isSupport;
}

//----------------------------------------------------------
// Filtering
//----------------------------------------------------------


bool BinaryExtension::filterSupport(Variable *dummy) {
    if(x->size() == 1) {
        for(int idvy : y->domain)
            if((*matrix)[x->domain[0]][idvy] == false && solver->delIdv(y, idvy) == false)
                return false;
        return solver->entail(this);
    }

    if(y->size() == 1) {
        for(int idvx : x->domain)
            if((*matrix)[idvx][y->domain[0]] == false && solver->delIdv(x, idvx) == false)
                return false;
        return solver->entail(this);
    }
    for(int idvx : reverse(x->domain)) {
        if(resx[idvx] != -1 && y->containsIdv(resx[idvx]) == true)
            continue;
        bool found = false;
        for(int idvy : y->domain) {
            if((*matrix)[idvx][idvy] == true) {
                resx[idvx] = idvy;
                resy[idvy] = idvx;
                found      = true;
                break;
            }
        }
        if(found == false && solver->delIdv(x, idvx) == false)
            return false;
    }
    for(int idvy : reverse(y->domain)) {
        if(resy[idvy] != -1 && x->containsIdv(resy[idvy]) == true)
            continue;
        bool found = false;
        for(int idvx : x->domain) {
            if((*matrix)[idvx][idvy] == true) {
                resy[idvy] = idvx;
                found      = true;
                break;
            }
        }
        if(found == false && solver->delIdv(y, idvy) == false)
            return false;
    }
    return true;
}


bool BinaryExtension::filterConflict(Variable *dummy) {
    if(x->size() == 1) {
        int idvx = x->domain[0];
        for(int idvy : y->domain)
            if((*matrix)[idvx][idvy] == true && solver->delIdv(y, idvy) == false)
                return false;
        return solver->entail(this);
    }
    if(y->size() == 1) {
        int idvy = y->domain[0];
        for(int idvx : x->domain)
            if((*matrix)[idvx][idvy] == true && solver->delIdv(x, idvx) == false)
                return false;
        return solver->entail(this);
    }

    if(existingX->size() < x->size()) {
        for(int idvx : (*existingX)) {
            if(x->containsIdv(idvx) == false)   // Just to avoid duplicate code
                continue;
            if(resx[idvx] != -1 && y->containsIdv(resx[idvx]) == true)
                continue;
            bool found = false;
            for(int idvy : y->domain) {
                if((*matrix)[idvx][idvy] == false) {
                    resx[idvx] = idvy;
                    resy[idvy] = idvx;
                    found      = true;
                    break;
                }
            }
            if(found == false && solver->delIdv(x, idvx) == false)
                return false;
        }
    } else {
        for(int idvx : x->domain) {
            if(resx[idvx] != -1 && y->containsIdv(resx[idvx]) == true)
                continue;
            bool found = false;
            for(int idvy : y->domain) {
                if((*matrix)[idvx][idvy] == false) {
                    resx[idvx] = idvy;
                    resy[idvy] = idvx;
                    found      = true;
                    break;
                }
            }
            if(found == false && solver->delIdv(x, idvx) == false)
                return false;
        }
    }


    if(existingY->size() < y->size()) {
        for(int idvy : *existingY) {
            if(y->containsIdv(idvy) == false)   // Just to avoid duplicate code
                continue;
            if(resy[idvy] != -1 && x->containsIdv(resy[idvy]) == true)
                continue;
            bool found = false;
            for(int idvx : x->domain) {
                if((*matrix)[idvx][idvy] == false) {
                    resy[idvy] = idvx;
                    found      = true;
                    break;
                }
            }
            if(found == false && solver->delIdv(y, idvy) == false)
                return false;
        }
    } else {
        for(int idvy : y->domain) {
            if(resy[idvy] != -1 && x->containsIdv(resy[idvy]) == true)
                continue;
            bool found = false;
            for(int idvx : x->domain) {
                if((*matrix)[idvx][idvy] == false) {
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

bool BinaryExtension::filter(Variable *x) {
    if(isSupport)
        return filterSupport(x);
    return filterConflict(x);
}
//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------

BinaryExtension::BinaryExtension(Problem &p, std::string n, bool support, Variable *xx, Variable *yy)
    : Extension(p, n, createScopeVec(xx, yy), 0, support),
      x(xx),
      y(yy),
      existingX(new vec<int>()),
      existingY(new vec<int>()),
      maxConflictsx(x->size() + 1),
      maxConflictsy(y->size() + 1),
      nbtuples(0) {
    matrix = new Matrix<bool>(x->domain.maxSize(), y->domain.maxSize());
    matrix->growTo(x->domain.maxSize());
    type = "Extension - Binary (" + std::to_string(isSupport) + ")";
}


BinaryExtension::BinaryExtension(Problem &p, std::string n, bool support, Variable *xx, Variable *yy,
                                 BinaryExtension *hasSameTuples)
    : Extension(p, n, createScopeVec(xx, yy), 0, support),
      x(xx),
      y(yy),
      existingX(hasSameTuples->existingX),
      existingY(hasSameTuples->existingX),
      maxConflictsx(x->size() + 1),
      maxConflictsy(y->size() + 1) {
    matrix   = hasSameTuples->matrix;
    nbtuples = hasSameTuples->nbtuples;
    type     = "Extension - Binary (" + std::to_string(isSupport) + ")";
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
        for(int i = 0; i < x->domain.maxSize(); i++) (*matrix)[i][idv2] = true;
        return;
    }
    if(idv2 == STAR) {
        for(int i = 0; i < y->domain.maxSize(); i++) (*matrix)[idv1][i] = true;
        return;
    }
    if(isSupport == false) {
        if(existingX->contains(idv1) == false)
            existingX->push(idv1);
        if(existingY->contains(idv2) == false)
            existingY->push(idv2);
    }
    (*matrix)[idv1][idv2] = true;
    nbtuples++;
}


void BinaryExtension::delayedConstruction(int id) {
    Constraint::delayedConstruction(id);
    resx.growTo(x->domain.maxSize(), -1);
    resy.growTo(y->domain.maxSize(), -1);
}


size_t BinaryExtension::nbTuples() { return nbtuples; }


struct CSPPropagation {   // x equal idv ??
    Variable *x;
    int       idv;
    bool      equal;
};
