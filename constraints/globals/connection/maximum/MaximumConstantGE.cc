#include "MaximumConstantGE.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool MaximumConstantGE::isSatisfiedBy(vec<int> &tuple) {
    int max = tuple[0];
    for(int i = 1; i < tuple.size(); i++)
        if(tuple[i] > max)
            max = tuple[i];
    return max >= k;
}

//----------------------------------------------
// Filtering
//----------------------------------------------

bool MaximumConstantGE::filter(Variable *dummy) {
    if(scope[sentinel1]->maximum() < k) {
        int i = 0;
        for(; i < scope.size(); i++)
            if(i != sentinel2 && scope[i]->maximum() >= k)
                break;
        if(i < scope.size())
            sentinel1 = i;
        else {
            if(scope[sentinel2]->maximum() < k)
                return false;

            solver->delValuesLowerOrEqualThan(scope[sentinel2], k - 1);
            solver->entail(this);
            return true;
        }
    }
    if(scope[sentinel2]->maximum() < k) {
        int i = 0;
        for(; i < scope.size(); i++)
            if(i != sentinel1 && scope[i]->maximum() >= k)
                break;
        if(i < scope.size())
            sentinel2 = i;
        else {
            assert(scope[sentinel1]->maximum() >= k);
            solver->delValuesLowerOrEqualThan(scope[sentinel1], k - 1);   // necessarily true returned
            solver->entail(this);
            return true;
        }
    }
    return true;
}
