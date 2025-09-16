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


bool BinaryExtensionSupport::filter(Variable *dummy) {
    if(x->size() == 1 && y->size() == 1) {
        if(supportsForX[x->domain[0]].contains(y->domain[0]) == false)
            return false;
        solver->entail(this);
        return true;
    }

    for(int idvx : x->domain) {
        if(resx[idvx] != -1 && y->containsIdv(resx[idvx]) == true)
            continue;
        int pos = 0;
        for(int idvy : supportsForX[idvx]) {
            if(y->containsIdv(idvy)) {
                resx[idvx] = idvy;
                resy[idvy] = idvx;
                break;
            }
            pos++;
        }
        if(pos == supportsForX[idvx].size() && solver->delIdv(x, idvx) == false)
            return false;
        if(pos == supportsForX[idvx].size() || supportsForX[idvx].size() == 0 || pos == 0)
            continue;
        assert(supportsForX[idvx].size() > 0);
        int tmp                 = supportsForX[idvx][0];
        supportsForX[idvx][0]   = supportsForX[idvx][pos];
        supportsForX[idvx][pos] = tmp;
    }

    for(int idvy : y->domain) {
        if(resy[idvy] != -1 && x->containsIdv(resy[idvy]) == true)
            continue;
        int pos = 0;
        for(int idvx : supportsForY[idvy]) {
            if(x->containsIdv(idvx)) {
                resy[idvy] = idvx;
                break;
            }
            pos++;
        }
        if(pos == supportsForY[idvy].size() && solver->delIdv(y, idvy) == false)
            return false;
        if(pos == supportsForY[idvy].size() || supportsForY[idvy].size() == 0 || pos == 0)
            continue;
        assert(supportsForY[idvy].size() > 0);

        int tmp                 = supportsForY[idvy][0];
        supportsForY[idvy][0]   = supportsForY[idvy][pos];
        supportsForY[idvy][pos] = tmp;
    }
    return true;
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