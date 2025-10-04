//
// Created by audemard on 04/10/2025.
//

#include "SumBoolean.h"

#include "Sum.h"
#include "SumEQ.h"
#include "constraints/Constraint.h"
#include "solver/Solver.h"

using namespace Cosoco;


//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool SumBooleanEQ::isSatisfiedBy(vec<int>& tuple) {
    int cnt = 0;
    for(int v : tuple)
        if(v == 1)
            cnt++;
    return cnt == limit;
}

bool SumBooleanLE::isSatisfiedBy(vec<int>& tuple) {
    int cnt = 0;
    for(int v : tuple)
        if(v == 1)
            cnt++;
    return cnt <= limit;
}

bool SumBoolean::isCorrectlyDefined() {
    for(Variable* x : scope)
        if(x->size() != 2)
            return false;
    return true;
}


//----------------------------------------------
// Filtering
//----------------------------------------------
bool SumBooleanEQ::filter(Variable* dummy) {
    int cnt0 = 0, cnt1 = 0;
    for(Variable* x : scope)
        if(x->size() == 1) {
            if(x->value() == 0)
                cnt0++;
            else
                cnt1++;
        }

    int diff = scope.size() - cnt0 - cnt1;
    if(cnt1 > limit || cnt1 + diff < limit)
        return false;
    if(cnt1 < limit && cnt1 + diff > limit)
        return true;
    // at this point, either cnt1 == limit, and we have to remove all 1s or cnt1 + diff == limit, and we have to remove all 0s
    int v = cnt1 == limit ? 1 : 0;
    for(Variable* x : scope) {
        if(x->size() != 1)
            solver->delVal(x, v);
    }
    return solver->entail(this);
}

bool SumBooleanLE::filter(Variable* dummy) {
    int cnt0 = 0, cnt1 = 0;
    for(Variable* x : scope)
        if(x->size() == 1) {
            if(x->value() == 0)
                cnt0++;
            else
                cnt1++;
        }
    if(cnt1 > limit)
        return false;

    int diff = scope.size() - cnt0 - cnt1;
    if(cnt1 + diff <= limit)
        return solver->entail(this);

    if(cnt1 == limit) {
        for(Variable* x : scope) {
            if(x->size() != 1)
                solver->assignToVal(x, 0);
        }
        return solver->entail(this);
    }
    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

SumBoolean::SumBoolean(Problem& p, std::string n, vec<Variable*>& vars, long l) : GlobalConstraint(p, n, "Sum", vars), limit(l) {
    isPostponable = true;
}

SumBooleanEQ::SumBooleanEQ(Problem& p, std::string n, vec<Variable*>& vars, long l) : SumBoolean(p, n, vars, l) {
    type = "Sum Boolean EQ";
}

SumBooleanLE::SumBooleanLE(Problem& p, std::string n, vec<Variable*>& vars, long l) : SumBoolean(p, n, vars, l) {
    type = "Sum Boolean LE";
}
