#include "constraints/globals/comparison/DistinctVectors.h"

#include "core/domain/DomainRange.h"
#include "solver/Solver.h"

using namespace Cosoco;
//----------------------------------------------------------
// check validity and correct definition
//----------------------------------------------------------

bool DistinctVectors::isSatisfiedBy(Cosoco::vec<int> &tuple) {
    vec<int> tupleX, tupleY;
    tupleX.growTo(size);
    tupleY.growTo(size);
    for(int i = 0; i < size; i++) {
        int posX  = toScopePosition(X[i]);
        tupleX[i] = tuple[posX];
    }
    for(int i = 0; i < size; i++) {
        int posY  = toScopePosition(Y[i]);
        tupleY[i] = tuple[posY];
    }
    for(int i = 0; i < size; i++)
        if(tupleX[i] != tupleY[i])
            return true;
    return false;
}


bool DistinctVectors::isCorrectlyDefined() {
    if(X.size() != Y.size())
        throw std::logic_error("Constraint " + std::to_string(idc) + ": DistinctVector: X and Y must have the same size");
    return true;
}


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool DistinctVectors::filter(Cosoco::Variable *x) {
    if(x == X[sentinel1] || x == Y[sentinel1]) {
        if(!isSentinel(sentinel1)) {
            int sentinel = findAnotherSentinel();
            if(sentinel != -1)
                sentinel1 = sentinel;
            else {
                if(X[sentinel2]->size() > 1 && Y[sentinel2]->size() > 1)
                    return true;
                else if(X[sentinel2]->size() == 1 && Y[sentinel2]->size() == 1)
                    return X[sentinel2]->value() != Y[sentinel2]->value();
                else
                    handlePossibleInferenceFor(sentinel2);
            }
        } else if(!isSentinel(sentinel2) && isPossibleInferenceFor(sentinel1))
            handlePossibleInferenceFor(sentinel1);
        return true;
    } else if(x == X[sentinel2] || x == Y[sentinel2]) {
        if(!isSentinel(sentinel2)) {
            int sentinel = findAnotherSentinel();
            if(sentinel != -1)
                sentinel2 = sentinel;
            else {
                if(X[sentinel1]->size() > 1 && Y[sentinel1]->size() > 1)
                    return true;
                else if(X[sentinel1]->size() == 1 && Y[sentinel1]->size() == 1)
                    return X[sentinel1]->value() != Y[sentinel1]->value();
                else
                    handlePossibleInferenceFor(sentinel1);
            }
        } else if(!isSentinel(sentinel1) && isPossibleInferenceFor(sentinel2))
            handlePossibleInferenceFor(sentinel2);
        return true;
    } else
        return true;
}


int DistinctVectors::findAnotherSentinel() {
    for(int i = 0; i < size; i++)
        if(i != sentinel1 && i != sentinel2 && isSentinel(i))
            return i;
    return -1;
}


bool DistinctVectors::isSentinel(int i) { return X[i]->size() > 1 || Y[i]->size() > 1 || X[i]->value() != Y[i]->value(); }


bool DistinctVectors::isPossibleInferenceFor(int sentinel) {
    return (X[sentinel]->size() == 1 && Y[sentinel]->size() > 1) || (X[sentinel]->size() > 1 && Y[sentinel]->size() == 1);
}


void DistinctVectors::handlePossibleInferenceFor(int sentinel) {
    assert(isPossibleInferenceFor(sentinel));
    // no wipe-out possible
    if(X[sentinel]->size() == 1)
        solver->delVal(Y[sentinel], X[sentinel]->value());
    else
        solver->delVal(X[sentinel], Y[sentinel]->value());
}


//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------


DistinctVectors::DistinctVectors(Cosoco::Problem &p, std::string n, vec<Variable *> &XX, vec<Variable *> &YY)
    : GlobalConstraint(p, n, "Distinct Vectors", Constraint::createScopeVec(&XX, &YY)) {
    XX.copyTo(X);
    YY.copyTo(Y);
    size      = X.size();
    sentinel1 = sentinel2 = 0;   // Must be initialized before looking for a new one
    sentinel1             = findAnotherSentinel();
    if(sentinel1 == -1)
        sentinel1 = 0;
    sentinel2 = findAnotherSentinel();
    if(sentinel2 == -1) {
        if(sentinel1 == 0)
            sentinel2 = 1;
        else
            sentinel2 = 0;
    }
    assert(sentinel1 != sentinel2);
}
