#include "SumNE.h"

#include "Sum.h"
#include "constraints/Constraint.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool SumNE::isSatisfiedBy(vec<int> &tuple) { return weightedSum(tuple) != limit; }

//----------------------------------------------
// Filtering
//----------------------------------------------


bool SumNE::filter(Variable *x) {
    if(watchedPosition1 == -1 || scope[watchedPosition1]->size() == 1)
        watchedPosition1 = findNewWatch();
    if(watchedPosition1 == -1) {   // No new non singleton variable
        if(watchedPosition2 == -1 || scope[watchedPosition2]->size() == 1)
            return computeSumExceptPos() != limit;

        return filterUniqueNonSingletonVariable(watchedPosition2);
    }
    if(watchedPosition2 == -1 || scope[watchedPosition2]->size() == 1)
        watchedPosition2 = findNewWatch();

    if(watchedPosition2 == -1)   // No new singleton variable
        filterUniqueNonSingletonVariable(watchedPosition1);
    return true;
}


int SumNE::findNewWatch() {
    for(int posx = 0; posx < scope.size(); posx++)
        if(posx != watchedPosition1 && posx != watchedPosition2 && scope[posx]->size() > 1)
            return posx;
    return -1;
}


long SumNE::computeSumExceptPos(int posx) {
    long sum = 0;
    for(int posy = 0; posy < scope.size(); posy++)
        if(posy != posx)
            sum += ((long)scope[posy]->value()) * coefficients[posy];
    return sum;
}


bool SumNE::filterUniqueNonSingletonVariable(int posx) {
    Variable *x   = scope[posx];
    long      sum = computeSumExceptPos(posx);

    for(int idv : reverse(x->domain))
        if(sum + x->domain.toVal(idv) * coefficients[posx] == limit)
            return solver->delIdv(x, idv);   // Only one value can be removed
    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

SumNE::SumNE(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &coefs, long l)
    : Sum(p, n, vars, coefs, l), watchedPosition1(-1), watchedPosition2(-1) { }
