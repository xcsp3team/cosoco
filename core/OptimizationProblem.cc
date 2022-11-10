#include "OptimizationProblem.h"
using namespace Cosoco;


OptimizationProblem::OptimizationProblem(const std::string &n) : Problem(n), objectiveLB(nullptr), objectiveUB(nullptr) { }


void OptimizationProblem::addObjectiveLB(ObjectiveConstraint *o, bool alreadyAdded) {
    if(objectiveLB != nullptr)
        throw std::logic_error("Lower Bound Objective is already specified");
    if(isConstructionDone)
        throw std::logic_error("Construction of the problem is already done! You can not add objective\n");
    objectiveLB = o;
    if(alreadyAdded == false)
        addConstraint(dynamic_cast<Constraint *>(o));
}


void OptimizationProblem::addObjectiveUB(ObjectiveConstraint *o, bool alreadyAdded) {
    if(objectiveUB != nullptr)
        throw std::logic_error("Upper Bound Objective is already specified");
    if(isConstructionDone)
        throw std::logic_error("Construction of the problem is already done! You can not add objective\n");
    objectiveUB = o;
    if(alreadyAdded == false)
        addConstraint(dynamic_cast<Constraint *>(o));
}
