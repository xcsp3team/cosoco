#include "CardinalityF.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

#define OFFSET(i) ((i) - offset)

bool CardinalityF::isSatisfiedBy(vec<int> &tuple) {
    /*clear();
    for(int v : tuple) {
        if(data.find(v) == data.end())
            continue;
        data[v].fixed++;
    }

    for(auto &x : data)
        if(x.second.fixed != x.second.occs)
            return false;
*/
    return true;
}


bool CardinalityF::isCorrectlyDefined() { return true; }

void CardinalityF::init() {
    valueToCompute.clear();
    for(int i = 0; i < values.size(); i++) {
        mandatories[i].clear();
        possibles[i].clear();
        valueToCompute.add(i);
    }

    for(int i = 0; i < vars.size(); i++) {
        Variable *x = vars[i];
        if(x->size() == 1) {
            int j = values2indexes[OFFSET(x->value())];
            if(j != -1)
                mandatories[j].add(i);
        } else {
            for(int idv : x->domain) {
                int j = values2indexes[OFFSET(x->domain.toVal(idv))];
                if(j != -1)
                    possibles[j].add(i);
            }
        }
    }
}

//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool CardinalityF::filter(Variable *dummy) {
    while(true) {
        int status = doFiltering();
        if(status == -1)
            return false;
        if(status == 0)
            break;
        for(int v : valueToCompute) {
            for(int posx : reverse(possibles[v])) {
                if(vars[posx]->containsValue(v) == false) {
                    possibles[v].del(posx);

                } else if(vars[posx]->isAssigned()) {
                    possibles[v].del(posx);
                    mandatories[v].add(posx);
                }
            }
        }
    }
    return true;
}

int CardinalityF::doFiltering() {
    init();
    bool again = false;
    for(int i : reverse(valueToCompute)) {
        int szBefore = occurs[i]->size();
        if(solver->delValuesLowerOrEqualThan(occurs[i], mandatories[i].size() - 1) == false)
            return -1;
        again |= (occurs[i]->size() < szBefore);

        szBefore = occurs[i]->size();
        if(solver->delValuesGreaterOrEqualThan(occurs[i], mandatories[i].size() + possibles[i].size() + 1) == false)
            return -1;
        again |= (occurs[i]->size() < szBefore);

        if(occurs[i]->isAssigned()) {
            if(possibles[i].size() + mandatories[i].size() == occurs[i]->value()) {
                for(int j : possibles[i]) {
                    mandatories[i].add(j);
                    again |= vars[j]->size() > 1;
                    if(solver->assignToVal(vars[j], values[i]) == false)
                        return -1;
                }
                possibles[i].clear();
                valueToCompute.del(i);   // value[i] restriction entailed
            } else if(mandatories[i].size() == occurs[i]->value()) {
                for(int j : possibles[i]) {
                    int szBefore = vars[j]->size();
                    if(solver->delVal(vars[j], values[i]) == false)
                        return -1;
                    again |= (vars[j]->size() < szBefore);
                }
                possibles[i].clear();
                valueToCompute.del(i);   // value[i] restriction entailed
            }
        }
    }

    return again;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

CardinalityF::CardinalityF(Problem &p, std::string n, vec<Variable *> &_vars, vec<int> &v, vec<Variable *> &o)
    : GlobalConstraint(p, n, "Cardinality F", Constraint::createScopeVec(&_vars, &o)) {
    isPostponable = true;
    _vars.copyTo(vars);
    o.copyTo(occurs);
    v.copyTo(values);
    valueToCompute.setCapacity(values.size(), false);
    possibles.growTo(values.size());
    mandatories.growTo(values.size());
    for(int i = 0; i < values.size(); i++) {
        mandatories[i].setCapacity(vars.size(), false);
        possibles[i].setCapacity(vars.size(), false);
    }
    int max = vars[0]->domain.maxSize();
    int min = vars[0]->domain.maxSize();
    for(Variable *x : vars) {
        if(x->maximum() > max)
            max = x->maximum();
        if(x->minimum() < min)
            min = x->minimum();
    }
    std::cout << min << " " << max << std::endl;
    values2indexes.growTo(max - min + 1, -1);
    offset  = min;
    int idx = 0;
    for(int v1 : values) values2indexes[OFFSET(v1)] = idx++;
}
