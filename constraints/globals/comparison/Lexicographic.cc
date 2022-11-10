#include "Lexicographic.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------------------
// check validity and correct definition
//----------------------------------------------------------

bool Lexicographic::isSatisfiedBy(vec<int> &tuple) {
    vec<int> tupleX, tupleY;
    tupleX.growTo(X.size());
    tupleY.growTo(Y.size());
    for(int i = 0; i < X.size(); i++) {
        int posX  = toScopePosition(X[i]);
        tupleX[i] = tuple[posX];
    }
    for(int i = 0; i < Y.size(); i++) {
        int posY  = toScopePosition(Y[i]);
        tupleY[i] = tuple[posY];
    }

    for(int i = 0; i < X.size(); i++) {
        if(tupleX[i] < tupleY[i])
            return true;
        if(tupleX[i] > tupleY[i])
            return false;
    }
    return !strict;
}


bool Lexicographic::isCorrectlyDefined() {
    if(X.size() != Y.size())
        throw std::logic_error("Constraint " + std::to_string(idc) + ": Lexicographic has X.size()!=Y.size() : " +
                               std::to_string(X.size()) + " " + std::to_string(Y.size()));
    return true;
}


//----------------------------------------------
// Filtering
//----------------------------------------------


bool Lexicographic::filter(Variable *dummy) {
    int alpha  = 0;
    int sz = X.size();
    while(alpha < sz) {   // Update alpha
        Variable *x = X[alpha], *y = Y[alpha];
        if(establishAC(x, y) == false)
            return false;
        if (x->size() == 1 && y->size() == 1) {
            if (x->value() < y->value()) {
                solver->entail(this);
                return true;
            }
            assert(x->value() == y->value());
            alpha++;
        } else {
            int minX = x->minimum(), minY = y->minimum();
            assert(minX <= minY);
            if (minX == minY && isConsistentPair(alpha, minX) == false && solver->delVal(y, minY) == false)
                    return false;
            int maxX = x->maximum(), maxY = y->maximum();
            assert(maxX <= maxY);
            if (maxX == maxY && isConsistentPair(alpha, maxX) == false && solver->delVal(x, maxX) == false)
                    return false;
            //assert (x->minimum() < y->maximum());
            return true;
        }
    }
    assert(alpha == sz);
    return !strict;

}


bool Lexicographic::establishAC(Variable *x, Variable *y) {
    int maxY = y->maximum();
    if(x->maximum() > maxY && solver->delValuesGreaterOrEqualThan(x, maxY + 1) == false)
        return false;

    int minX = x->minimum();
    if(y->minimum() < minX && solver->delValuesLowerOrEqualThan(y, minX - 1) == false)
        return false;

    return true;
}


bool Lexicographic::isConsistentPair(int alpha, int v) {
    time++;
    int sz = X.size();
    Variable *x = X[alpha], *y = Y[alpha];

    setTime(toScopePosition(x), v);
    setTime(toScopePosition(y), v);

    for (int i = alpha + 1; i < sz; i++) {
        int posx = toScopePosition(X[i]);
        int posy = toScopePosition(Y[i]);
        int minx = times[posx] == time ? vals[posx] : X[i]->minimum();
        int maxy = times[posy] == time ? vals[posy] : Y[i]->maximum();
        if (minx < maxy)
            return true;
        if (minx > maxy)
            return false;
        setTime(posx, minx);
        setTime(posy, maxy);
    }
    return !strict;
}


void Lexicographic::setTime(int posx, int v) {
    assert(posx >= 0 && posx < times.size());
    assert(posx >= 0 && posx < vals.size());

    times[posx] = time;
    vals[posx]  = v;
}

//----------------------------------------------
// Constructor and initialisation methods
//----------------------------------------------


Lexicographic::Lexicographic(Problem &p, std::string n, vec<Variable *> &XX, vec<Variable *> &YY, bool st)
    : GlobalConstraint(p, n, "Lex", 2 * XX.size()), strict(st), time(0) {
    XX.copyTo(X);
    YY.copyTo(Y);

    vec<Variable *> vars;
    for(Variable *x : XX) x->fake = 0;
    for(Variable *y : YY) y->fake = 0;
    // Put variables only once in scope!!
    for(Variable *x : XX) {
        if(x->fake == 0)
            vars.push(x);
        x->fake = 1;
    }
    for(Variable *y : YY) {
        if(y->fake == 0)
            vars.push(y);
        y->fake = 1;
    }


    scopeInitialisation(vars);
    times.growTo(scope.size(), 0);
    vals.growTo(scope.size(), 0);
}
