#include "MinimumConstantLE.h"

#include "solver/Solver.h"
using namespace Cosoco;


//----------------------------------------------
// Check validity
//----------------------------------------------

bool MinimumConstantLE::isSatisfiedBy(vec<int>& tuple) { return tuple.min() <= k; }


//----------------------------------------------
// Filtering
//----------------------------------------------

bool MinimumConstantLE::filter(Variable* dummy) {
    if(scope[sentinel1]->minimum() > k) {
        int i = 0;
        for(; i < scope.size(); i++)
            if(i != sentinel2 && scope[i]->minimum() <= k)
                break;
        if(i < scope.size())
            sentinel1 = i;
        else {
            if(scope[sentinel2]->minimum() > k)
                return false;
            solver->delValuesGreaterOrEqualThan(scope[sentinel2], k + 1);   // necessarily true returned
            solver->entail(this);
            return true;
        }
    }
    if(scope[sentinel2]->minimum() > k) {
        int i = 0;
        for(; i < scope.size(); i++)
            if(i != sentinel1 && scope[i]->minimum() <= k)
                break;
        if(i < scope.size())
            sentinel2 = i;
        else {
            assert(scope[sentinel1]->minimum() <= k);
            solver->delValuesGreaterOrEqualThan(scope[sentinel1], k + 1);   // necessarily true returned
            solver->entail(this);
            return true;
        }
    }
    return true;
}