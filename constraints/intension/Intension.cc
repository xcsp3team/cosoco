#include "constraints/intension/Intension.h"

#ifdef USE_XCSP3
#include "XCSP3Tree.h"


using namespace Cosoco;


//----------------------------------------------------------
// check validity
//----------------------------------------------------------

bool Intension::isSatisfiedBy(vec<int> &t) {
    for(int i = 0; i < scope.size(); i++) tuple[scope[i]->_name] = t[i];
    return evaluator->evaluate(tuple);
}

//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool Intension::filter(Variable *dummy) {
    for(Variable *x : scope) {
        if(x->size() > 1)
            return true;
        assert(x->size() == 1);
        tuple[x->_name] = x->value();
    }
    return evaluator->evaluate(tuple);
}

//----------------------------------------------------------
// Objective functions
//----------------------------------------------------------

// The constraint is something like : le(expr,bound) or ge(expr, bound)


void Intension::updateBound(long bound) {
    auto *nodeB = dynamic_cast<XCSP3Core::NodeBinary *>(evaluator->root);
    auto *nodeC = dynamic_cast<XCSP3Core::NodeConstant *>(nodeB->parameters[1]);
    nodeC->val  = bound;
}


long Intension ::maxUpperBound() { return INT_MAX - 10; }


long Intension::minLowerBound() { return INT_MIN + 10; }


long Intension::computeScore(vec<int> &solution) {
    assert(solution.size() == scope.size());
    for(int i = 0; i < scope.size(); i++) tuple[scope[i]->_name] = solution[i];
    auto *nodeB = dynamic_cast<XCSP3Core::NodeBinary *>(evaluator->root);
    return nodeB->parameters[0]->evaluate(tuple);
}


void Intension::display(bool d) { std::cout << evaluator->toString() << std::endl; }

#endif /* USE_XCSP3 */