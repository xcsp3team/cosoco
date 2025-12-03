#include "ElementVariable.h"

#include <XCSP3Constants.h>

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool ElementVariable::isSatisfiedBy(vec<int> &tuple) {
    return true;
    int idx = posIndex == -1 ? tuple[tuple.size() - 2] : tuple[posIndex];
    int res = tuple[tuple.size() - 1];
    return tuple[idx - (startAtOne ? 1 : 0)] == res;
}


bool ElementVariable::isCorrectlyDefined() {
    if(index == value)
        throw std::logic_error("Constraint " + std::to_string(idc) + ": ElementVariable has index=result");
    return Element::isCorrectlyDefined();
}

//----------------------------------------------
// Filtering
//----------------------------------------------


bool ElementVariable::filter(Variable *dummy) {
    if(index->size() > 1) {
        if(filterValue() == false)
            return false;
        while(true) {
            // updating idom (and indexSentinels)
            int sizeBefore = index->size();
            if(filterIndex() == false)
                return false;
            if(sizeBefore == index->size())
                break;
            // updating vdom (and valueSentinels)
            sizeBefore = value->size();
            if(filterValue() == false)
                return false;
            if(sizeBefore == value->size())
                break;
        }
    }
    // If index is singleton, we update dom(list[index]) and vdom so that they are both equal to the
    // intersection of the two domains
    if(index->size() == 1) {
        Variable *x = list[index->value()];
        if(solver->delValuesNotInDomain(x, value->domain) == false)
            return false;
        if(solver->delValuesNotInDomain(value, x->domain) == false)
            return false;
        if(value->size() == 1)
            solver->entail(this);
    }
    return true;
}

bool ElementVariable::validIndex(int posx) {
    int v = indexSentinels[posx];
    if(v != STAR && list[posx]->containsValue(v) && value->containsValue(v))
        return true;
    for(int idv2 : list[posx]->domain) {   // int a = dom.first(); a != -1; a = dom.next(a)) {
        int v2 = list[posx]->domain.toVal(idv2);
        if(value->containsValue(v2)) {
            indexSentinels[posx] = v2;
            return true;
        }
    }
    return false;
}

bool ElementVariable::filterIndex() {
    for(int idv : index->domain) {
        int v = index->domain.toVal(idv);
        if((v < 0 || v >= list.size() || validIndex(v) == false) && solver->delIdv(index, idv) == false)
            return false;
    }
    return true;
}

bool ElementVariable::validValue(int idv) {
    int v        = value->domain.toVal(idv);
    int sentinel = valueSentinels[idv];
    if(sentinel != -1 && index->containsValue(sentinel) && list[sentinel]->containsValue(v))
        return true;
    for(int idv2 : index->domain) {
        int v2 = index->domain.toVal(idv2);
        if(v2 >= 0 && v2 < list.size() && list[v2]->containsValue(v)) {
            valueSentinels[idv] = v2;
            return true;
        }
    }
    return false;
}

bool ElementVariable::filterValue() {
    if(value->size() > 100)
        return true;
    for(int idv : value->domain)
        if(validValue(idv) == false && solver->delIdv(value, idv) == false)
            return false;
    return true;
}


//----------------------------------------------
// Constructor and initialisation methods
//----------------------------------------------

ElementVariable::ElementVariable(Problem &p, std::string n, vec<Variable *> &vars, Variable *i, Variable *r, bool one)
    : Element(p, n, "Element Variable", Constraint::createScopeVec(&vars, i, r), i, one), value(r) {
    szVector = vars.size();
    posIndex = vars.firstOccurrenceOf(i);
    vars.copyTo(list);
    valueSentinels.growTo(value->domain.maxSize(), -1);
    indexSentinels.growTo(vars.size(), STAR);
    isPostponable = true;
}
