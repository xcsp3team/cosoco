//
// Created by audemard on 27/03/2026.
//

#include "Knapsack.h"

#include "solver/Solver.h"

using namespace Cosoco;

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

int sum(vec<int> &tuple, vec<int> &coefs) {
    int sum = 0;
    for(int posx = 0; posx < coefs.size(); posx++) {
        sum += coefs[posx] * tuple[posx];
    }
    return sum;
}

bool Knapsack::isSatisfiedBy(vec<int> &tuple) { return sum(tuple, weights) <= wlimit && sum(tuple, profits) >= plimit; }

bool KnapsackVARW::isSatisfiedBy(vec<int> &tuple) { return sum(tuple, weights) <= tuple.last() && sum(tuple, profits) >= plimit; }

bool KnapsackVARP::isSatisfiedBy(vec<int> &tuple) { return sum(tuple, weights) <= wlimit && sum(tuple, profits) >= tuple.last(); }
bool KnapsackVARWP::isSatisfiedBy(vec<int> &tuple) {
    return sum(tuple, weights) <= tuple[tuple.size() - 2] && sum(tuple, profits) >= tuple.last();
}

bool Knapsack::isCorrectlyDefined() {
    return weights.size() == vars.size() && profits.size() == vars.size() && weights.min() >= 0 && profits.min() >= 0;
}

//----------------------------------------------
// Filrtering
//----------------------------------------------
void Knapsack::recomputeBounds() {
    wmin = wmax = pmin = pmax = 0;
    for(int posx = 0; posx < scope.size(); posx++) {
        wmin += weights[posx] * scope[posx]->minimum();
        wmax += weights[posx] * scope[posx]->maximum();
        pmin += profits[posx] * scope[posx]->minimum();
        pmax += profits[posx] * scope[posx]->maximum();
    }
}

int computeLimit(int limit, int coeff, bool LE) {
    int newLimit = limit / coeff;
    if(LE == false && limit % coeff != 0)
        newLimit++;
    return newLimit;
}


bool Knapsack::filter(Variable *dummy) {
    if(basic)
        recomputeBounds();
    if(wmax <= wlimit && pmin >= plimit)
        return basic ? solver->entail(this) : true;
    if(wmin > wlimit || pmax < plimit)
        return false;
    if(wmax > wlimit) {   // otherwise nothing to do
        for(int posx = 0; posx < vars.size(); posx++) {
            Variable *x = vars[posx];
            if(x->size() == 1 || weights[posx] == 0)
                continue;
            int wcoeff = weights[posx];
            int pcoeff = profits[posx];
            wmax -= wcoeff * x->maximum();
            pmax -= pcoeff * x->maximum();
            long wmini = wmin - wcoeff * x->minimum();   // we remove the contribution of the variable we consider
            long pmini = pmin - pcoeff * x->minimum();

            // solver->delValuesGreaterOrEqualThan(x, computeLimit(wlimit - wmini + 1, wcoeff, false));

            if(profits[posx] == 0)
                continue;

            while(true) {
                int v              = x->maximum();
                int nPossibleMoves = (int)std::floor((wlimit - wmini - wcoeff * v) / minWeight);   // 1 if 0 present ? why ?
                if((pmini + nPossibleMoves * maxProfit) + pcoeff * v < plimit) {                   //
                    if(solver->delVal(x, v) == false)
                        return false;
                } else
                    break;
            }
            wmax += wcoeff * x->maximum();
            pmax += pcoeff * x->maximum();
            if(wmax <= wlimit)
                break;
        }
    }
    if(pmin < plimit) {   // otherwise nothing to do
        for(int posx = 0; posx < vars.size(); posx++) {
            Variable *x = vars[posx];
            if(x->size() == 1 || profits[posx] == 0)
                continue;
            int wcoeff = weights[posx];
            int pcoeff = profits[posx];
            wmin -= wcoeff * x->minimum();
            pmin -= pcoeff * x->minimum();
            long wmaxi = wmax - wcoeff * x->maximum();   // we remove the contribution of the variable we consider
            long pmaxi = pmax - pcoeff * x->maximum();

            // solver->delValuesLowerOrEqualThan(x, computeLimit(plimit - pmaxi - 1, pcoeff, true) - 1);
            if(weights[posx] == 0)
                continue;
            while(true) {
                int v              = x->minimum();
                int nPossibleMoves = (int)std::floor((pmaxi + pcoeff * v - plimit) / minProfit);   // 1 if 0 present ? why ?
                if((wmaxi - nPossibleMoves * maxWeight) + wcoeff * v > wlimit) {
                    if(solver->delVal(x, v) == false)
                        return false;
                } else
                    break;
            }
            wmin += wcoeff * x->minimum();
            pmin += pcoeff * x->minimum();
            if(pmin >= plimit)
                break;
        }
    }
    return true;
}


bool KnapsackVARW::filter(Variable *dummy) {
    recomputeBounds();
    if(solver->delValuesLowerOrEqualThan(varWLimit, wmin - 1) == false)
        return false;
    wlimit = varWLimit->maximum();
    return Knapsack::filter(dummy);
}


bool KnapsackVARP::filter(Variable *dummy) {
    recomputeBounds();
    if(solver->delValuesGreaterOrEqualThan(varPLimit, pmax + 1) == false)
        return false;
    plimit = varPLimit->minimum();
    return Knapsack::filter(dummy);
}

bool KnapsackVARWP::filter(Variable *dummy) {
    recomputeBounds();
    if(solver->delValuesLowerOrEqualThan(varWLimit, wmin - 1) == false)
        return false;
    if(solver->delValuesGreaterOrEqualThan(varPLimit, pmax + 1) == false)
        return false;
    wlimit = varWLimit->maximum();
    plimit = varPLimit->minimum();
    return Knapsack::filter(dummy);
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

Knapsack::Knapsack(Problem &p, std::string n, vec<Variable *> &_vars, vec<int> &_w, vec<int> &_p, int wl, int pl)
    : GlobalConstraint(p, n, "Knapsack", _vars) {
    _vars.copyTo(vars);
    _w.copyTo(weights);
    _p.copyTo(profits);
    wlimit        = wl;
    plimit        = pl;
    basic         = true;
    minWeight     = weights.min();
    maxWeight     = weights.max();
    minProfit     = profits.min();
    maxProfit     = profits.max();
    isPostponable = true;
}

KnapsackVARW::KnapsackVARW(Problem &p, std::string n, vec<Variable *> &_vars, vec<int> &_w, vec<int> &_p, Variable *wl, int pl)
    : Knapsack(p, n, _vars, _w, _p, 0, pl) {
    addToScope(wl);
    varWLimit = wl;
    basic     = false;
    type      = "Knapsack VW";
}

KnapsackVARP::KnapsackVARP(Problem &p, std::string n, vec<Variable *> &_vars, vec<int> &_w, vec<int> &_p, int wl, Variable *pl)
    : Knapsack(p, n, _vars, _w, _p, wl, 0) {
    addToScope(pl);
    varPLimit = pl;
    basic     = false;
    type      = "Knapsack VP";
}

KnapsackVARWP::KnapsackVARWP(Problem &p, std::string n, vec<Variable *> &_vars, vec<int> &_w, vec<int> &_p, Variable *wl,
                             Variable *pl)
    : Knapsack(p, n, _vars, _w, _p, 0, 0) {
    addToScope(wl);
    addToScope(pl);
    std::cout << scope.size() << std::endl;
    varPLimit = pl;
    varWLimit = wl;
    basic     = false;
    type      = "KnapsackVWP";
}