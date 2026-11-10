#include "constraints/intension/Intension.h"

#include <constraints/genericFiltering/TupleIteratorWithoutOrder.h>


//----------------------------------------------------------
// check validity
//----------------------------------------------------------

bool Cosoco::Intension::isSatisfiedBy(vec<int> &t) {
    for(int i = 0; i < scope.size(); i++) tuple[problem.variableName(scope[i])] = t[i];
    return evaluator->evaluate(tuple);
}

//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool Cosoco::Intension::filter(Variable *dummy) {
    for(Variable *x : scope) {
        if(x->size() > 1)
            return true;
        assert(x->size() == 1);
        tuple[problem.variableName(x)] = x->value();
    }
    return evaluator->evaluate(tuple);
}

//----------------------------------------------------------
// Objective functions
//----------------------------------------------------------

// The constraint is something like : le(expr,bound) or ge(expr, bound)


void Cosoco::Intension::updateBound(long bound) {
    auto *nodeB = dynamic_cast<NodeBinary *>(evaluator->root);
    auto *nodeC = dynamic_cast<NodeConstant *>(nodeB->parameters[1]);
    nodeC->val  = bound;
}


long Cosoco::Intension::maxUpperBound() { return INT_MAX - 10; }


long Cosoco::Intension::minLowerBound() { return INT_MIN + 10; }


long Cosoco::Intension::computeScore(vec<int> &solution) {
    assert(solution.size() == scope.size());
    for(int i = 0; i < scope.size(); i++) tuple[problem.variableName(scope[i])] = solution[i];
    auto *nodeB = dynamic_cast<NodeBinary *>(evaluator->root);
    return nodeB->parameters[0]->evaluate(tuple);
}


void Cosoco::Intension::display(bool d) { std::cout << evaluator->toString() << std::endl; }