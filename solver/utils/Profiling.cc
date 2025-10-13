#include "Profiling.h"

#include <iomanip>

#include "solver/Solver.h"
#include "utils/System.h"

using namespace Cosoco;

Profiling::Profiling(Solver *s) : solver(s) { }

void Profiling::initialize() {
    for(Constraint *c : solver->problem.constraints)
        if(constraintsData.find(c->type) == constraintsData.end())
            constraintsData.insert(make_pair(c->type, ConstraintData()));
    constraintsData.insert(std::make_pair("nogoods", ConstraintData()));
}

bool Profiling::beforeConstraintCall(Constraint *c) {
    currentTime = realTime();
    return true;
}

bool Profiling::afterConstraintCall(Constraint *c, int nbDeletedVariables) {
    std::string ct = c == nullptr ? "nogoods" : c->type;
    constraintsData[ct].totalTime += (realTime() - currentTime);
    constraintsData[ct].nbCalls++;
    constraintsData[ct].uselessCalls += (nbDeletedVariables > 0) ? 0 : 1;
    return true;
}

void Profiling::display() {
    printf("\n\n");
    printf("c  ------ Profiling ----- \n");
    std::cout << "c                       ";
    std::cout << "  Time";
    std::cout << "                Calls";
    std::cout << "               perCalls";
    std::cout << "            Useless\n";


    for(auto &pair : constraintsData) {
        std::cout << "c ";
        std::cout << std::left << std::setw(22) << std::setfill(' ') << pair.first << ": ";
        std::cout << std::left << std::setw(20) << std::setfill(' ') << pair.second.totalTime;
        std::cout << std::left << std::setw(20) << std::setfill(' ') << pair.second.nbCalls;
        std::cout << std::left << std::setw(20) << std::setfill(' ')
                  << (static_cast<double>(pair.second.totalTime) / static_cast<double>(pair.second.nbCalls));
        std::cout << std::left << std::setw(20) << std::setfill(' ')
                  << (1.0 * pair.second.uselessCalls / pair.second.nbCalls) * 100;
        std::cout << "\n";
    }
    printf("c  ---------------------- \n\n");
}