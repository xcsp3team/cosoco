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

bool SumBooleanLE::isSatisfiedBy(vec<int>& tuple) { return sum(tuple) <= limit; }

bool SumBooleanGE::isSatisfiedBy(vec<int>& tuple) { return sum(tuple) >= limit; }

bool SumBooleanGenLE::isSatisfiedBy(vec<int>& tuple) {
    return true;   // TODO
}

bool SumBooleanGenGE::isSatisfiedBy(vec<int>& tuple) {
    return true;   // TODO
}

bool SumBooleanGenEQ::isSatisfiedBy(vec<int>& tuple) {
    return true;   // TODO
}

bool SumBoolean::isCorrectlyDefined() {
    for(Variable* x : scope)
        if(x->size() != 2)
            return false;
    return true;
}

bool SumBooleanGen::isCorrectlyDefined() { return nodes.size() == scope.size(); }


//----------------------------------------------
// Filtering
//----------------------------------------------

long SumBoolean::sum(vec<int>& tuple) {
    long sum = 0;
    for(int v : tuple) sum += v;
    return sum;
}

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

bool SumBooleanGE::filter(Variable* dummy) {
    int cnt0 = 0, cnt1 = 0;
    for(Variable* x : scope)
        if(x->size() == 1) {
            if(x->value() == 0)
                cnt0++;
            else
                cnt1++;
        }

    int diff = scope.size() - cnt0 - cnt1;

    if(cnt1 >= limit)
        return solver->entail(this);


    if(cnt1 + diff < limit)
        return false;

    if(cnt1 + diff == limit) {
        for(Variable* x : scope) {
            if(x->size() != 1)
                solver->assignToVal(x, 1);
        }
        return solver->entail(this);
    }
    return true;
}

bool SumBooleanGenLE::filter(Variable* dummy) {
    int cnt0 = 0, cnt1 = 0;
    for(BasicNode* x : nodes)
        if(x->size() == 1) {
            if(x->maximum() == 0)   // Min or Max .. Use value ??
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
        for(BasicNode* x : nodes) {
            if(x->size() != 1)
                x->setFalse(solver);
        }
        return solver->entail(this);
    }
    return true;
}

bool SumBooleanGenGE::filter(Variable* dummy) {
    int cnt0 = 0, cnt1 = 0;
    for(BasicNode* x : nodes)
        if(x->size() == 1) {
            if(x->minimum() == 0)
                cnt0++;
            else
                cnt1++;
        }

    int diff = scope.size() - cnt0 - cnt1;

    if(cnt1 >= limit)
        return solver->entail(this);


    if(cnt1 + diff < limit)
        return false;

    if(cnt1 + diff == limit) {
        for(BasicNode* x : nodes) {
            if(x->size() != 1)
                x->setTrue(solver);
        }
        return solver->entail(this);
    }
    return true;
}

bool SumBooleanGenEQ::filter(Variable* dummy) {
    int cnt0 = 0, cnt1 = 0;
    for(BasicNode* x : nodes)
        if(x->size() == 1) {
            if(x->minimum() == 0)
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

    for(BasicNode* x : nodes) {
        if(x->size() != 1) {
            if(cnt1 == limit)
                x->setFalse(solver);
            else
                x->setTrue(solver);
        }
    }
    return solver->entail(this);
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

SumBooleanGE::SumBooleanGE(Problem& p, std::string n, vec<Variable*>& vars, long l) : SumBoolean(p, n, vars, l) {
    type = "Sum Boolean GE";
}

SumBooleanGen::SumBooleanGen(Problem& p, std::string n, vec<BasicNode*>& _nodes, vec<Variable*>& vars, long l)
    : GlobalConstraint(p, n, "Sum", vars), limit(l) {
    isPostponable = true;
    _nodes.copyTo(nodes);
}

SumBooleanGenLE::SumBooleanGenLE(Problem& p, std::string n, vec<BasicNode*>& _nodes, vec<Variable*>& vars, long l)
    : SumBooleanGen(p, n, _nodes, vars, l) {
    type = "Sum Boolean Gen LE";
}

SumBooleanGenGE::SumBooleanGenGE(Problem& p, std::string n, vec<BasicNode*>& _nodes, vec<Variable*>& vars, long l)
    : SumBooleanGen(p, n, _nodes, vars, l) {
    type = "Sum Boolean Gen GE";
}

SumBooleanGenEQ::SumBooleanGenEQ(Problem& p, std::string n, vec<BasicNode*>& _nodes, vec<Variable*>& vars, long l)
    : SumBooleanGen(p, n, _nodes, vars, l) {
    type = "Sum Boolean Gen EQ";
}

//----------------------------------------------
// Objective constraint
//----------------------------------------------

void SumBoolean::updateBound(long bound) { limit = bound; }   // Update the current bound

long SumBoolean::maxUpperBound() {
    long max = 0;
    for(Variable* x : scope) max += x->maximum();
    return max;
}


long SumBoolean::minLowerBound() {
    long min = 0;
    for(Variable* x : scope) min += x->minimum();
    return min;
}


long SumBoolean::computeScore(vec<int>& solution) { return sum(solution); }

void SumBooleanGen::updateBound(long bound) { limit = bound; }   // Update the current bound

long SumBooleanGen::maxUpperBound() {
    long max = 0;
    for(BasicNode* x : nodes) max += x->maximum();
    return max;
}


long SumBooleanGen::minLowerBound() {
    long min = 0;
    for(BasicNode* x : nodes) min += x->minimum();
    return min;
}

long SumBooleanGen::computeScore(vec<int>& solution) {
    long sum = 0;
    for(int i = 0; i < nodes.size(); i++) sum += nodes[i]->value(solution[i]);
    return sum;
}