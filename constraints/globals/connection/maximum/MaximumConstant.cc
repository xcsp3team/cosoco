#include "MaximumConstant.h"

using namespace Cosoco;


//----------------------------------------------
// Objective constraint
//----------------------------------------------

void MaximumConstant::updateBound(long bound) { k = (int)bound; }


long MaximumConstant::maxUpperBound() {
    int nb = scope[0]->domain.maximum();
    for(int i = 1; i < scope.size(); i++)
        if(scope[i]->maximum() > nb)
            nb = scope[i]->maximum();
    return nb;
}


long MaximumConstant::minLowerBound() {
    int nb = scope[0]->minimum();
    for(int i = 1; i < scope.size(); i++)
        if(scope[i]->minimum() < nb)
            nb = scope[i]->minimum();
    return nb;
}


long MaximumConstant::computeScore(vec<int> &solution) {
    long nb = solution[0];
    for(int s : solution)
        if(s > nb)
            nb = s;
    return nb;
}
