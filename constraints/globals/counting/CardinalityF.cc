#include "CardinalityF.h"

#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

#define OFFSET(i) ((i) - offset)

bool AbstractCardinalityF::isSatisfiedBy(vec<int> &tuple) {
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


bool AbstractCardinalityF::isCorrectlyDefined() { return true; }

//----------------------------------------------
// Filtering
//----------------------------------------------


void AbstractCardinalityF::init(bool full) {
    if(full)
        valueToCompute.clear();
    for(int i = 0; i < values.size(); i++) {
        mandatories[i].clear();
        possibles[i].clear();
        if(full)
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


bool AbstractCardinalityF::filter(Variable *dummy) {
    init(true);
    while(true) {
        int status = doFiltering();
        if(status == -1)
            return false;
        if(status == 0)
            break;
        init(false);
    }
    return true;
}

int CardinalityF::doFiltering() {
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
                    again |= vars[j]->size() > 1;
                    if(solver->assignToVal(vars[j], values[i]) == false)
                        return -1;
                }
            } else if(mandatories[i].size() == occurs[i]->value()) {
                for(int j : possibles[i]) {
                    int szBefore = vars[j]->size();
                    if(solver->delVal(vars[j], values[i]) == false)
                        return -1;
                    again |= (vars[j]->size() < szBefore);
                }
            }
        }
    }

    return again;
}


int CardinalityInt::doFiltering() {
    bool again = false;
    for(int i : reverse(valueToCompute)) {
        if(mandatories[i].size() > occurs[i])
            return -1;
        if(mandatories[i].size() + possibles[i].size() < occurs[i])
            return -1;


        if(possibles[i].size() + mandatories[i].size() == occurs[i]) {
            if(possibles[i].size() > 0)
                valueToCompute.del(i);

            for(int j : possibles[i]) {
                again |= vars[j]->size() > 1;
                if(solver->assignToVal(vars[j], values[i]) == false)
                    return -1;
            }
        } else if(mandatories[i].size() == occurs[i]) {
            valueToCompute.del(i);
            for(int j : possibles[i]) {
                int szBefore = vars[j]->size();
                if(solver->delVal(vars[j], values[i]) == false)
                    return -1;
                again |= (vars[j]->size() < szBefore);
            }
        }
    }
    return again;
}

//----------------------------------------------
// Construction and initialisation
//----------------------------------------------


AbstractCardinalityF::AbstractCardinalityF(Problem &p, std::string n, vec<Variable *> &_vars, vec<int> &v)
    : GlobalConstraint(p, n, "Cardinality F", 0) {
    isPostponable = true;
    _vars.copyTo(vars);
    addToScope(vars);
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

CardinalityF::CardinalityF(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &v, vec<Variable *> &o)
    : AbstractCardinalityF(p, n, vars, v) {
    o.copyTo(occurs);
    addToScope(o);
}

CardinalityInt::CardinalityInt(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &v, vec<int> &o)
    : AbstractCardinalityF(p, n, vars, v) {
    for(int i : values) std::cout << i << " ";

    o.copyTo(occurs);
}