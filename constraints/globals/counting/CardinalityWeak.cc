#include "CardinalityWeak.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool CardinalityWeak::isSatisfiedBy(vec<int> &tuple) {
    clear();
    for(int v : tuple) {
        if(data.find(v) == data.end())
            continue;
        data[v].fixed++;
    }

    for(auto &x : data)
        if(x.second.fixed != x.second.occs)
            return false;
    return true;
}


bool CardinalityWeak::isCorrectlyDefined() { return true; }

void CardinalityWeak::clear() {
    for(auto &x : data) {
        x.second.fixed = 0;
    }
}

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool CardinalityWeak::filter(Variable *x) {
    clear();
    vec<int> valuesToDelete;
    for(Variable *x : scope) {
        if(x->size() > 1)
            continue;
        data[x->value()].fixed++;
        if(data[x->value()].occs < data[x->value()].fixed)
            return false;
        if(data[x->value()].occs == data[x->value()].fixed)
            valuesToDelete.push(x->value());
    }

    for(int v : valuesToDelete)
        for(Variable *x : scope)
            if(x->size() > 1 && solver->delVal(x, v) == false)
                return false;

    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

CardinalityWeak::CardinalityWeak(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &v, vec<int> &o)
    : GlobalConstraint(p, n, "Cardinality Weak", vars) {
    for(int i = 0; i < v.size(); i++) {
        CardData tmp = {o[i], 0};
        data.insert(std::make_pair(v[i], tmp));
    }
    isPostponable = true;
}
