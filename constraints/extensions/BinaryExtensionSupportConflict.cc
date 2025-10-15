#include "BinaryExtensionSupportConflict.h"

#include <utility>

#include "Extension.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------------------
// check validity and correct definition
//----------------------------------------------------------


bool BinaryExtensionConflict::isSatisfiedBy(vec<int> &tuple) {
    int idvx = x->domain.toIdv(tuple[0]);
    int idvy = y->domain.toIdv(tuple[1]);
    if(supportsForX[idvx].maxSize() == 0)
        return true;
    return supportsForX[idvx].contains(idvy) == false;
}

//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool BinaryExtensionConflict::filter(Variable *dummy) {
    if(x->size() == 1) {
        for(int idvy : supportsForX[x->domain[0]]) {
            if(solver->delIdv(y, idvy) == false)
                return false;
        }
        return solver->entail(this);
    }
    if(y->size() == 1) {
        for(int idvx : supportsForY[y->domain[0]]) {
            if(solver->delIdv(x, idvx) == false)
                return false;
        }
        return solver->entail(this);
    }

    bool someX = false;
    for(int idvx : inConflictsX) {
        if(x->containsIdv(idvx) == false)
            continue;
        someX = true;
        if(resx[idvx] != -1 && y->containsIdv(resx[idvx]) == true)
            continue;
        bool found = false;
        for(int idvy : y->domain)
            if(supportsForX[idvx].contains(idvy) == false) {
                found      = true;
                resx[idvx] = idvy;
                break;
            }
        if(found == false && solver->delIdv(x, idvx) == false)
            return false;
    }
    if(someX == false)
        return solver->entail(this);

    bool someY = false;
    for(int idvy : inConflictsY) {
        if(y->containsIdv(idvy) == false)
            continue;
        someY = true;
        if(resy[idvy] != -1 && x->containsIdv(resy[idvy]) == true)
            continue;
        bool found = false;
        for(int idvx : x->domain)
            if(supportsForY[idvy].contains(idvx) == false) {
                found      = true;
                resy[idvy] = idvx;
                break;
            }
        if(found == false && solver->delIdv(y, idvy) == false)
            return false;
    }
    if(someY == false)
        return solver->entail(this);

    return true;
}


//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------

BinaryExtensionConflict::BinaryExtensionConflict(Problem &p, std::string n, Variable *xx, Variable *yy)
    : Extension(p, n, createScopeVec(xx, yy), 0, false), x(xx), y(yy), nbtuples(0) {
    type = "Extension - Binary Conflict";
    supportsForX.growTo(x->domain.maxSize());
    supportsForY.growTo(y->domain.maxSize());
    resx.growTo(x->domain.maxSize(), -1);
    resy.growTo(y->domain.maxSize(), -1);
}


BinaryExtensionConflict::BinaryExtensionConflict(Problem &p, std::string n, Variable *xx, Variable *yy,
                                                 BinaryExtensionConflict *hasSameTuples)
    : Extension(p, n, createScopeVec(xx, yy), 0, false), x(xx), y(yy) {
    nbtuples = hasSameTuples->nbtuples;
    supportsForX.growTo(xx->domain.maxSize());
    supportsForY.growTo(yy->domain.maxSize());
    for(int i = 0; i < hasSameTuples->supportsForX.size(); i++) {
        if(hasSameTuples->supportsForX[i].maxSize() > 0)
            supportsForX[i].setCapacity(hasSameTuples->supportsForX[i].maxSize(), false);
        for(int idv : hasSameTuples->supportsForX[i]) supportsForX[i].add(idv);
    }

    for(int i = 0; i < hasSameTuples->supportsForY.size(); i++) {
        if(hasSameTuples->supportsForY[i].maxSize() > 0)
            supportsForY[i].setCapacity(hasSameTuples->supportsForY[i].maxSize(), false);
        for(int idv : hasSameTuples->supportsForY[i]) supportsForY[i].add(idv);
    }

    for(int idv : hasSameTuples->inConflictsX) inConflictsX.insert(idv);
    for(int idv : hasSameTuples->inConflictsY) inConflictsY.insert(idv);

    resx.growTo(x->domain.maxSize(), -1);
    resy.growTo(y->domain.maxSize(), -1);

    type = "Extension - Binary Conflict";
}


void BinaryExtensionConflict::addTuple(vec<int> &tupleIdv) {
    // Check if tuples are inside domains
    assert(tupleIdv[0] != STAR && tupleIdv[1] != STAR);
    if((tupleIdv[0] > x->domain.maxSize()) || tupleIdv[0] < 0)
        return;
    if((tupleIdv[1] > y->domain.maxSize()) || tupleIdv[1] < 0)
        return;
    addTuple(tupleIdv[0], tupleIdv[1]);
}


void BinaryExtensionConflict::addTuple(int idv1, int idv2) {
    assert(idv1 >= 0 && idv1 < x->domain.maxSize());
    assert(idv2 >= 0 && idv2 < y->domain.maxSize());
    if(supportsForX[idv1].maxSize() == 0)
        supportsForX[idv1].setCapacity(y->domain.maxSize(), false);
    supportsForX[idv1].add(idv2);
    if(supportsForY[idv2].maxSize() == 0)
        supportsForY[idv2].setCapacity(x->domain.maxSize(), false);
    supportsForY[idv2].add(idv1);

    inConflictsX.insert(idv1);
    inConflictsY.insert(idv2);

    nbtuples++;
}


size_t BinaryExtensionConflict::nbTuples() { return nbtuples; }