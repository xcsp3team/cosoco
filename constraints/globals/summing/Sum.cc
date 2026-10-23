//
// Created by audemard on 21/07/2026.
//
#include "Sum.h"
using namespace Cosoco;

long SimpleSum::sum(vec<int>& tuple) {
    long sum = 0;
    for(int v : tuple) sum += static_cast<long>(v);
    return sum;
}

long WeightedSum::weightedSum(vec<int>& tuple) {
    long sum = 0;
    for(int i = 0; i < tuple.size(); i++) sum += coefficients[i] * static_cast<long>(tuple[i]);
    return sum;
}

bool WeightedSum::isCorrectlyDefined() {
    if(scope.size() != coefficients.size())
        throw std::logic_error("Constraint " + std::to_string(idc) + ": Sum, list and coefficients mus have same size");

    for(int i = 1; i < coefficients.size(); i++) {
        if(coefficients[i] == 0)
            throw std::logic_error("Constraint " + std::to_string(idc) + ": Sum, a coefficient is 0");
        if(coefficients[i - 1] > coefficients[i]) {
            throw std::logic_error("Constraint " + std::to_string(idc) + ": Sum needs order on coefficients");
        }
    }
    return true;
}

void WeightedSum::assignVariable(Variable* x, int posx) {
    Constraint::assignVariable(x, posx);
    long coeff = coefficients[posx];
    fixed += coeff * x->value();
}

void WeightedSum::unassignVariable(Variable* x, int posx, int v) {
    Constraint::unassignVariable(x, posx, v);
    long coeff = coefficients[posx];
    fixed -= coeff * v;
}

void SimpleSum::assignVariable(Variable* x, int posx) {
    Constraint::assignVariable(x, posx);
    fixed += x->value();
}

void SimpleSum::unassignVariable(Variable* x, int posx, int v) {
    Constraint::unassignVariable(x, posx, v);
    fixed -= v;
}


void SimpleSum::computeBounds() {
    min = fixed;
    max = fixed;
    for(int posx : unassignedVariablesIdx) {
        Variable* x = scope[posx];
        min += x->minimum();
        max += x->maximum();
    }
}

void WeightedSum::computeBounds() {
    min = fixed;
    max = fixed;
    for(int posx : unassignedVariablesIdx) {
        Variable* x     = scope[posx];
        long      xmin  = x->minimum();
        long      xmax  = x->maximum();
        long      coeff = coefficients[posx];
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
