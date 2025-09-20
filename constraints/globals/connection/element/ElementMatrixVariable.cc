//
// Created by audemard on 12/09/2025.
//

#include "ElementMatrixVariable.h"

#include "Solver.h"
using namespace Cosoco;

//----------------------------------------------------------
// check validity
//----------------------------------------------------------

bool ElementMatrixVariable::isSatisfiedBy(vec<int> &tuple) {
    return true;   // TODO
    int i = tuple[rindexPosition], j = tuple[cindexPosition];
    int v = tuple[vPosition];
    return tuple[i * matrix.size() + j] == v;
}


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool ElementMatrixVariable::validRowIndex(int i) {
    int j = rindexColSentinels[i], v = rindexValSentinels[i];
    if(j != -1 && cindex->containsIdv(j) && matrix[rindex->domain.toVal(i)][cindex->domain.toVal(j)]->containsValue(v) &&
       value->containsValue(v))
        return true;
    for(int idv1 : cindex->domain) {
        Variable *x = matrix[rindex->domain.toVal(i)][cindex->domain.toVal(idv1)];
        for(int idv2 : x->domain) {
            int va = x->domain.toVal(idv2);
            if(value->containsValue(va)) {
                rindexColSentinels[i] = idv1;
                rindexValSentinels[i] = va;
                return true;
            }
        }
    }
    return false;
}

bool ElementMatrixVariable::validColIndex(int j) {
    int i = cindexRowSentinels[j], v = cindexValSentinels[j];
    if(i != -1 && rindex->containsIdv(i) && matrix[rindex->domain.toVal(i)][cindex->domain.toVal(j)]->containsValue(v) &&
       value->containsValue(v))
        return true;
    for(int idv1 : rindex->domain) {
        Variable *x = matrix[rindex->domain.toVal(idv1)][cindex->domain.toVal(j)];
        for(int idv2 : x->domain) {
            int va = x->domain.toVal(idv2);
            if(value->containsValue(va)) {
                cindexRowSentinels[j] = i;
                cindexValSentinels[j] = va;
                return true;
            }
        }
    }
    return false;
}


bool ElementMatrixVariable::validValue(int a) {
    int va = value->domain.toVal(a);
    int i = valueRowSentinels[a], j = valueColSentinels[a];
    if(i != -1 && rindex->containsIdv(i) && cindex->containsIdv(j) &&
       matrix[rindex->domain.toVal(i)][cindex->domain.toVal(j)]->containsValue(va))
        return true;
    for(int idvr : rindex->domain)
        for(int idvc : cindex->domain) {
            if(matrix[rindex->domain.toVal(idvr)][cindex->domain.toVal(idvc)]->containsValue(va)) {
                valueRowSentinels[a] = idvr;
                valueColSentinels[a] = idvc;
                return true;
            }
        }
    return false;
}


bool ElementMatrixVariable::filterIndex() {
    for(int idvr : rindex->domain)
        if(validRowIndex(idvr) == false && solver->delIdv(rindex, idvr) == false)
            return false;
    for(int idvc : cindex->domain)
        if(validColIndex(idvc) == false && solver->delIdv(cindex, idvc) == false)
            return false;
    return true;
}

bool ElementMatrixVariable::filterValue() {
    for(int idv : value->domain)
        if(validValue(idv) == false && solver->delIdv(value, idv) == false)
            return false;
    return true;
}


bool ElementMatrixVariable::filter(Variable *x) {
    // If indexes are not both singleton, we try to prune values :
    // - in vdom, we prune the values which are not in any of the domains of the list variables
    // - in rdom and cdom, we prune the values that cannot lead to any value in vdom
    if(rindex->size() > 1 || cindex->size() > 1) {
        // updating vdom (and some sentinels)
        if(filterValue() == false)
            return false;
        while(true) {
            // updating rdom,and cdom (and some sentinels)
            int sizeBefore = rindex->size() + cindex->size();
            if(filterIndex() == false)
                return false;
            if(sizeBefore == rindex->size() + cindex->size())
                break;
            // updating vdom (and some sentinels)
            sizeBefore = value->size();
            if(filterValue() == false)
                return false;
            if(sizeBefore == value->size())
                break;
        }
    }
    // If indexes are both singleton, we enforce value to the corresponding cell of the matrix
    if(rindex->size() == 1 && cindex->size() == 1) {
        Variable *x = matrix[rindex->value()][cindex->value()];
        if(solver->delValuesNotInDomain(x, value->domain) == false)
            return false;
        if(solver->delValuesNotInDomain(value, x->domain) == false)
            return false;
        if(value->size() == 1)
            solver->entail(this);
    }
    return true;
}


//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------

ElementMatrixVariable::ElementMatrixVariable(Problem &p, std::string nn, vec<vec<Variable *> > &mm, Variable *ri, Variable *ci,
                                             Variable *v)
    : ElementMatrix(p, nn) {
    type  = "Element Matrix Variable";
    value = v;

    matrix.growTo(mm.size());
    vec<Variable *> vars;
    for(int i = 0; i < mm.size(); i++) {
        for(int j = 0; j < mm[i].size(); j++) {
            matrix[i].push(mm[i][j]);
            vars.push(mm[i][j]);
        }
    }
    rindex = ri;
    cindex = ci;

    rindexPosition = vars.firstOccurrenceOf(rindex);
    cindexPosition = vars.firstOccurrenceOf(cindex);
    vPosition      = vars.firstOccurrenceOf(value);
    if(rindexPosition == -1) {
        vars.push(rindex);
        rindexPosition = vars.size() - 1;
    }

    if(cindexPosition == -1) {
        vars.push(cindex);
        cindexPosition = vars.size() - 1;
    }

    if(vPosition == -1) {
        vars.push(value);
        vPosition = vars.size() - 1;
    }

    addToScope(vars);


    int n = matrix.size(), m = matrix[0].size(), d = value->domain.maxSize();

    rindexColSentinels.growTo(n, -1);
    rindexValSentinels.growTo(n, -1);
    cindexRowSentinels.growTo(m, -1);
    cindexValSentinels.growTo(m, -1);
    valueRowSentinels.growTo(d, -1);
    valueColSentinels.growTo(d, -1);
}