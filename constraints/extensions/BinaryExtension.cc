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
    return supportsForX[idvx].contains(idvy);
}

//----------------------------------------------------------
// Filtering
//----------------------------------------------------------


bool BinaryExtension::filter(Variable *dummy) {
    if(x->size() == 1 && y->size() == 1) {
        if(supportsForX[x->domain[0]].contains(y->domain[0]) == false)
            return false;
        solver->entail(this);
        return true;
    }

    for(int idvx : x->domain) {
        int pos = 0;
        std::cout << " X=" << idvx << ":  ";
        for(int idvy : supportsForX[idvx]) std::cout << idvy << " ";
        std::cout << "\n";


        for(int idvy : supportsForX[idvx]) {
            //  std::cout << " " << idvy << std::endl;
            if(y->containsIdv(idvy))
                break;
            pos++;
        }
        if(pos == supportsForX[idvx].size() && solver->delIdv(x, idvx) == false)
            return false;
        if(supportsForX[idvx].size() == 0 || pos == 0)
            continue;
        assert(supportsForX[idvx].size() > 0);
        int tmp                 = supportsForX[idvx][0];
        supportsForX[idvx][0]   = supportsForX[idvx][pos];
        supportsForX[idvx][pos] = tmp;
    }

    for(int idvy : y->domain) {
        std::cout << " Y=" << idvy << ":  ";
        for(int idvx : supportsForY[idvy]) std::cout << idvx << " ";
        std::cout << "\n";

        int pos2 = 0;
        for(int idvx : supportsForY[idvy]) {
            if(x->containsIdv(idvx))
                break;
            pos2++;
        }
        std::cout << pos2 << " " << supportsForY[idvy].size() << std::endl;
        if(pos2 == supportsForY[idvy].size() && solver->delIdv(y, idvy) == false)
            return false;
        if(supportsForY[idvy].size() == 0 || pos2 == 0)
            continue;
        assert(supportsForY[idvy].size() > 0);

        int tmp                  = supportsForY[idvy][0];
        supportsForY[idvy][0]    = supportsForY[idvy][pos2];
        supportsForY[idvy][pos2] = tmp;
    }
    return true;
}


//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------

BinaryExtension::BinaryExtension(Problem &p, std::string n, bool support, Variable *xx, Variable *yy)
    : Extension(p, n, createScopeVec(xx, yy), 0, support), x(xx), y(yy), nbtuples(0) {
    supportsForX.growTo(x->domain.maxSize());
    supportsForY.growTo(y->domain.maxSize());
}


BinaryExtension::BinaryExtension(Problem &p, std::string n, bool support, Variable *xx, Variable *yy,
                                 BinaryExtension *hasSameTuples)
    : Extension(p, n, createScopeVec(xx, yy), 0, support), x(xx), y(yy) {
    supportsForX.growTo(xx->domain.maxSize());
    supportsForY.growTo(yy->domain.maxSize());
    for(int i = 0; i < hasSameTuples->supportsForX.size(); i++) hasSameTuples->supportsForX[i].copyTo(supportsForX[i]);
    for(int i = 0; i < hasSameTuples->supportsForY.size(); i++) hasSameTuples->supportsForY[i].copyTo(supportsForY[i]);
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
        for(int i = 0; i < x->domain.maxSize(); i++) supportsForY[idv2].push(i);
        return;
    }
    if(idv2 == STAR) {
        for(int i = 0; i < y->domain.maxSize(); i++) supportsForX[idv1].push(i);
        return;
    }
    std::cout << idv1 << " " << idv2 << std::endl;
    assert(idv1 >= 0 && idv1 <= supportsForX.size());
    assert(idv2 >= 0 && idv2 <= supportsForY.size());
    supportsForX[idv1].push(idv2);
    supportsForY[idv2].push(idv1);
    nbtuples++;
}


size_t BinaryExtension::nbTuples() { return nbtuples; }


struct CSPPropagation {   // x equal idv ??
    Variable *x;
    int       idv;
    bool      equal;
};
