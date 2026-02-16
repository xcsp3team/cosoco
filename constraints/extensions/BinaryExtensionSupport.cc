#include "BinaryExtensionSupport.h"

#include <utility>

#include "Extension.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------------------
// check validity and correct definition
//----------------------------------------------------------


bool BinaryExtensionSupport::isSatisfiedBy(vec<int> &tuple) {
    int idvx = x->domain.toIdv(tuple[0]);
    int idvy = y->domain.toIdv(tuple[1]);
    return supportsForX[idvx].contains(idvy);
}

//----------------------------------------------------------
// Filtering
//----------------------------------------------------------
bool BinaryExtensionSupport::filterOn(Variable *xx, Variable *yy, vec<vec<int>> &supportsForXX, vec<vec<int>> &supportsForYY,
                                      vec<int> &resXX, vec<int> &resYY) {
    if(xx->size() == 1) {
        vec<bool> appears;
        appears.growTo(yy->domain.maxSize(), false);
        for(int i : supportsForXX[xx->domain[0]]) appears[i] = true;

        for(int idvy : yy->domain) {
            if(appears[idvy] == false && solver->delIdv(yy, idvy) == false)
                return false;
        }
        return solver->entail(this);
    }

    for(int idvx : xx->domain) {
        if(resXX[idvx] != -1 && yy->containsIdv(resXX[idvx]) == true)
            continue;
        int pos = 0;
        for(int idvy : supportsForXX[idvx]) {
            if(yy->containsIdv(idvy)) {
                resXX[idvx] = idvy;
                resYY[idvy] = idvx;
                break;
            }
            pos++;
        }
        if(pos == supportsForXX[idvx].size() && solver->delIdv(xx, idvx) == false)
            return false;
        if(pos == supportsForXX[idvx].size() || supportsForXX[idvx].size() == 0 || pos == 0)
            continue;
        assert(supportsForXX[idvx].size() > 0);
        int tmp                  = supportsForXX[idvx][0];
        supportsForXX[idvx][0]   = supportsForXX[idvx][pos];
        supportsForXX[idvx][pos] = tmp;
    }
    return true;
}

bool BinaryExtensionSupport::filter(Variable *dummy) {
    if(x->size() == 1) {
        for(int idv : supportsForX[x->domain[0]])
            if(solver->delIdv(y, idv) == false)
                return false;
        return solver->entail(this);
    }
    if(y->size() == 1) {
        for(int idv : supportsForY[y->domain[0]])
            if(solver->delIdv(x, idv) == false)
                return false;
        return solver->entail(this);
    }


    if(filterOn(x, y, supportsForX, supportsForY, resx, resy) == false)
        return false;
    return filterOn(y, x, supportsForY, supportsForX, resy, resx);
}


//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------

BinaryExtensionSupport::BinaryExtensionSupport(Problem &p, std::string n, bool support, Variable *xx, Variable *yy)
    : Extension(p, n, createScopeVec(xx, yy), 0, support), x(xx), y(yy), nbtuples(0) {
    supportsForX.growTo(x->domain.maxSize());
    supportsForY.growTo(y->domain.maxSize());
    type = "Extension - Binary Support";
    resx.growTo(x->domain.maxSize(), -1);
    resy.growTo(y->domain.maxSize(), -1);
}


BinaryExtensionSupport::BinaryExtensionSupport(Problem &p, std::string n, bool support, Variable *xx, Variable *yy,
                                               BinaryExtensionSupport *hasSameTuples)
    : Extension(p, n, createScopeVec(xx, yy), 0, support), x(xx), y(yy) {
    supportsForX.growTo(xx->domain.maxSize());
    supportsForY.growTo(yy->domain.maxSize());
    for(int i = 0; i < hasSameTuples->supportsForX.size(); i++) hasSameTuples->supportsForX[i].copyTo(supportsForX[i]);
    for(int i = 0; i < hasSameTuples->supportsForY.size(); i++) hasSameTuples->supportsForY[i].copyTo(supportsForY[i]);
    nbtuples = hasSameTuples->nbtuples;
    type     = "Extension - Binary Support";
    resx.growTo(x->domain.maxSize(), -1);
    resy.growTo(y->domain.maxSize(), -1);
}


void BinaryExtensionSupport::addTuple(vec<int> &tupleIdv) {
    // Check if tuples are inside domains
    if((tupleIdv[0] != STAR && tupleIdv[0] > x->domain.maxSize()) || tupleIdv[0] < 0)
        return;
    if((tupleIdv[1] != STAR && tupleIdv[1] > y->domain.maxSize()) || tupleIdv[1] < 0)
        return;
    addTuple(tupleIdv[0], tupleIdv[1]);
}


void BinaryExtensionSupport::addTuple(int idv1, int idv2) {
    assert(idv1 >= 0 && (idv1 == STAR || idv1 < x->domain.maxSize()));
    assert(idv2 >= 0 && (idv2 == STAR || idv2 < y->domain.maxSize()));
    if(idv1 == STAR) {
        for(int i = 0; i < x->domain.maxSize(); i++) {
            supportsForY[idv2].push(i);
            supportsForX[i].push(idv2);
        }
        nbtuples += x->domain.maxSize();
        return;
    }
    if(idv2 == STAR) {
        for(int i = 0; i < y->domain.maxSize(); i++) {
            supportsForX[idv1].push(i);
            supportsForY[i].push(idv1);
        }
        nbtuples += y->domain.maxSize();
        return;
    }
    supportsForX[idv1].push(idv2);
    supportsForY[idv2].push(idv1);
    nbtuples++;
}


size_t BinaryExtensionSupport::nbTuples() { return nbtuples; }