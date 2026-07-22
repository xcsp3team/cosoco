//
// Created by audemard on 21/07/2026.
//
#include "Sum.h"
using namespace Cosoco;

long SimpleSum::sum(vec<int>& tuple) {
    long sum = 0;
    for(int i = 0; i < tuple.size(); i++) sum += static_cast<long>(tuple[i]);
    return sum;
}

long WeightedSum::weightedSum(vec<int>& tuple) {
    long sum = 0;
    for(int i = 0; i < tuple.size(); i++) sum += coefficients[i] * static_cast<long>(tuple[i]);
    return sum;
}

void SimpleSum::computeBounds() {
    min = max = 0;
    for(int i = 0; i < scope.size(); i++) {
        Variable* x = scope[i];
        min += x->minimum();
        max += x->maximum();
    }
}

void WeightedSum::computeBounds() {
    min = max = 0;
    for(int i = 0; i < scope.size(); i++) {
        Variable* x     = scope[i];
        long      xmin  = x->minimum();
        long      xmax  = x->maximum();
        long      coeff = coefficients[i];
        min += coeff * (coeff >= 0 ? xmin : xmax);
        max += coeff * (coeff >= 0 ? xmax : xmin);
    }
}

bool WeightedSum::delWRTOrder(Variable* x, long l, int coeff, XCSP3Core::OrderType order) {
    if(order == XCSP3Core::LT) {
        order = XCSP3Core::LE;
        l--;
    } else if(order == XCSP3Core::GT) {
        type = XCSP3Core::GE;
        l++;
    }
    if(coeff < 0) {
        coeff = -coeff;
        order = order == XCSP3Core::LE ? XCSP3Core::GE : XCSP3Core::LE;
        l     = -l;
    }
    long newLimit = (std::abs(l) / coeff) * (l < 0 ? -1 : 1);
    if(l > 0 && order == XCSP3Core::GE && l % coeff != 0)
        newLimit++;
    if(l < 0 && order == XCSP3Core::LE && -l % coeff != 0)
        newLimit--;
    return order == XCSP3Core::LE ? solver->delValuesLE(x, newLimit) : solver->delValuesGE(x, newLimit);
}
