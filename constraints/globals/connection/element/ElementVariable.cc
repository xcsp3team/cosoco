#include "ElementVariable.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool ElementVariable::isSatisfiedBy(vec<int> &tuple) {
    int idx = posIndex == -1 ? tuple[tuple.size() - 2] : posIndex;
    int res = tuple[tuple.size() - 1];
    return true;
    return tuple[idx - (startAtOne ? 1 : 0)] == res;
}


bool ElementVariable::isCorrectlyDefined() {
    if(index == result)
        throw std::logic_error("Constraint " + std::to_string(idc) + ": ElementVariable has index=result");
    return Element::isCorrectlyDefined();
}

//----------------------------------------------
// Filtering
//----------------------------------------------

bool ElementVariable::filter(Variable *x) {
    // If index is not singleton, we try to prune values :
    // - in result's domain, we prune the values which aren't in any of vector variables' domains
    // - in index's domain, we prune the values v for which there is no value j such that vector[v] and result both have j in
    // their domains
    if(index->size() > 1) {   // Update vectorSentinels and index
        if(reduceResultDomain() == false)
            return false;
        while(true) {
            int szBefore = index->size();
            for(int idvIndex : reverse(index->domain)) {
                int sentinel = vectorSentinels[idvIndex];
                if(sentinel == -1 || getVariableFor(idvIndex)->containsValue(sentinel) == false ||
                   result->containsValue(sentinel) == false) {
                    if(findVectorSentinelFor(idvIndex) == false && solver->delIdv(index, idvIndex) == false)
                        return false;
                }
            }

            if(szBefore == index->size())
                break;

            szBefore = result->size();
            if(reduceResultDomain() == false)
                return false;
            if(szBefore == result->size())
                break;
        }
    }
    // If index is singleton, we update dom(vector[index]) and dom(result) so that they are both equal to the intersection of the
    // two domains
    if(index->size() == 1) {
        Variable *x = getVariableFor(index->value());
        if(x->size() == 1) {
            if(result->containsValue(x->value()) == false)
                return false;
        } else {
            if(solver->delValuesNotInDomain(x, result->domain) == false)
                return false;
        }
        if(solver->delValuesNotInDomain(result, x->domain) == false)
            return false;
        if(x->size() == 1)
            solver->entail(this);
    }
    return true;
}


bool ElementVariable::findVectorSentinelFor(int posIndex) {
    Variable *x = getVariableFor(posIndex);
    for(int idv : x->domain) {
        int v = x->domain.toVal(idv);
        if(result->containsValue(v)) {
            vectorSentinels[posIndex] = v;
            return true;
        }
    }
    return false;
}


bool ElementVariable::findResultSentinelFor(int idvResult) {
    int v  = result->domain.toVal(idvResult);
    int sz = index->size();
    for(int i = 0; i < sz; i++) {
        int idx = index->domain[i];
        if(getVariableFor(idx)->containsValue(v)) {
            resultSentinels[idvResult] = idx;
            return true;
        }
    }
    return false;
}


bool ElementVariable::reduceResultDomain() {
    for(int idvResult : reverse(result->domain)) {
        int sentinel = resultSentinels[idvResult];
        if(sentinel == -1 || index->containsIdv(sentinel) == false ||
           getVariableFor(sentinel)->containsValue(result->domain.toVal(idvResult)) == false) {
            if(findResultSentinelFor(idvResult) == false && solver->delIdv(result, idvResult) == false)
                return false;
        }
    }
    return true;
}


//----------------------------------------------
// Constructor and initialisation methods
//----------------------------------------------

ElementVariable::ElementVariable(Problem &p, std::string n, vec<Variable *> &vars, Variable *i, Variable *r, bool one)
    : Element(p, n, "Element Variable", Constraint::createScopeVec(&vars, i, r), i, one), result(r) {
    szVector = vars.size();
    posIndex = vars.firstOccurrenceOf(i);
    resultSentinels.growTo(result->domain.maxSize(), -1);
    vectorSentinels.growTo(vars.size(), -1);
}
