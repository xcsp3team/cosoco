#include "MinimumConstant.h"

using namespace Cosoco;


void MinimumConstant::updateBound(long bound) { k = (int)bound; }

long MinimumConstant::maxUpperBound() {
    int nb = scope[0]->maximum();
    for(int i = 1; i < scope.size(); i++)
        if(scope[i]->maximum() > nb)
            nb = scope[i]->maximum();
    return nb;
}

long MinimumConstant::minLowerBound() {
    int nb = scope[0]->minimum();
    for(int i = 1; i < scope.size(); i++)
        if(scope[i]->minimum() < nb)
            nb = scope[i]->minimum();
    return nb;
}

long MinimumConstant::computeScore(vec<int> &solution) {
    long nb = solution[0];
    for(int s : solution)
        if(s < nb)
            nb = s;
    return nb;
}
