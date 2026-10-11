#include "DistinctVectors.h"

#include "DomainRange.h"
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
        int posX  = variablePosition.toScopePosition(X[i]->idx);
        tupleX[i] = tuple[posX];
    }
    for(int i = 0; i < size; i++) {
        int posY  = variablePosition.toScopePosition(Y[i]->idx);
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

bool DistinctVectorsK::isCorrectlyDefined() {
    for(int i = 0; i < lists.size(); i++) {
        for(int j = i + 1; j < lists.size(); j++)
            if(lists[i].size() != lists[j].size())
                throw std::logic_error("Constraint " + std::to_string(idc) +
                                       ": DistinctVector K: Two lists have different sizes");
    }
    if(scope.size() != n_rows * n_cols)
        throw std::logic_error("Constraint " + std::to_string(idc) + ": DistinctVector K: all variables must be different");


    return true;
}

bool DistinctVectorsK::areDifferent(int start1, int start2, vec<int> &tuple) const {
    for(int j = 0; j < n_cols; j++)
        if(tuple[start1 + j] != tuple[start2 + j])
            return true;
    return false;
}


bool DistinctVectorsK::isSatisfiedBy(Cosoco::vec<int> &tuple) {
    for(int i = 0; i < n_rows; i++)
        for(int ii = i + 1; ii < n_rows; ii++)
            if(areDifferent(i * n_cols, ii * n_cols, tuple) == false)
                return false;
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


int DistinctVectorsK::isSentinelFor(int i, int ii, int j) {
    Variable *x = lists[i][j];
    Variable *y = lists[ii][j];
    if(x->size() == 1 && y->size() == 1)
        return x->value() == y->value() ? 0 : 1;

    sentinels[i][ii] = j;
    sentinels[ii][i] = j;
    return -1;
}

int DistinctVectorsK::findSentinel(int i, int ii, int jToIgnore) {
    int j = sentinels[i][ii];
    if(j != jToIgnore) {
        int b = isSentinelFor(i, ii, j);
        if(b != 0)
            return b;
    }
    for(j = 0; j < n_cols; j++) {
        if(j == jToIgnore)
            continue;
        int b = isSentinelFor(i, ii, j);
        if(b != 0)
            return b;
    }
    return 0;
}

bool DistinctVectorsK::filter(Variable *x) {
    if(x->size() != 1)
        return true;
    int level = solver->decisionLevel();
    int v     = x->value();
    int p     = variablePosition.toScopePosition(x->idx);
    int i = rows[p], j = cols[p];
    for(int ii = 0; ii < n_rows; ii++) {
        if(ii == i)
            continue;
        int k = offsets[std::min(i, ii)] + std::abs(ii - i) - 1;

        if(set.contains(k) == false)
            continue;

        Variable *y = lists[ii][j];
        if(y->containsValue(v) == false) {
            set.del(k, level);
            continue;
        }
        int b = findSentinel(i, ii, j);
        if(b == 0) {   // no other sentinel
            if(solver->delVal(y, v) == false)
                return false;
            set.del(k, level);
        } else if(b == 1) {   // strong sentinel
            set.del(k, level);
        }
    }
    return true;
}

//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------
void DistinctVectorsK::notifyDeleteDecision(Variable *x, int v, Solver &s, bool isFull) {
    set.restoreLimit(s.decisionLevel() + 1);
}


DistinctVectors::DistinctVectors(Cosoco::Problem &p, vec<Variable *> &XX, vec<Variable *> &YY)
    : GlobalConstraint(p, "Distinct Vectors", Constraint::createScopeVec(&XX, &YY)) {
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

DistinctVectorsK::DistinctVectorsK(Problem &p, vec<vec<Variable *>> &XX) : GlobalConstraint(p, "Distinct Vectors K", 0) {
    lists.growTo(XX.size());
    int i = 0;
    for(auto &list : XX) list.copyTo(lists[i++]);
    n_rows = lists.size();
    n_cols = lists[0].size();

    for(auto &list : lists) addToScope(list);
}


void DistinctVectors::delayedConstruction(int id) {
    Constraint::delayedConstruction(id);
    variablePosition.makeDelayedConstruction(this);
}


void DistinctVectorsK::attachSolver(Solver *s) {
    Constraint::attachSolver(s);
    s->addObserverDeleteDecision(this);   // We need to restore validTuples.
}

void DistinctVectorsK::delayedConstruction(int id) {
    Constraint::delayedConstruction(id);
    variablePosition.makeDelayedConstruction(this);

    rows = new int[scope.size()];
    cols = new int[scope.size()];
    for(int i = 0; i < n_rows; i++)
        for(int j = 0; j < n_cols; j++) {
            int p   = variablePosition.toScopePosition(lists[i][j]->idx);
            rows[p] = i;
            cols[p] = j;
        }
    offsets    = new int[n_rows - 1];
    offsets[0] = 0;
    for(int i = 1; i < n_rows - 1; i++) offsets[i] = offsets[i - 1] + (n_rows - i);
    sentinels = new int *[n_rows];
    for(int i = 0; i < n_rows; i++) {
        sentinels[i] = new int[n_rows];
        std::fill_n(sentinels[i], n_rows, 0);
    }
    set.setCapacity((n_rows * (n_rows - 1)) / 2, true);   // TODO
}