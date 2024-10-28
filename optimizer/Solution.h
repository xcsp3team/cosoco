#ifndef COSOCO_SOLUTION_H
#define COSOCO_SOLUTION_H

#include <iostream>
#include <map>
#include <mutex>
#include <vector>

#include "Termcolor.h"
#include "core/OptimizationProblem.h"
#include "mtl/Vec.h"
#include "utils/Constants.h"
#include "utils/System.h"


namespace Cosoco {
class Solution {   // This class comes from pseudo boolean competition and was provided by O. Roussel.
    friend class Optimizer;

   private:
    std::vector<int> *tmp, *preserved;
    long              bound;
    bool              invertBestCost;
    std::mutex        mutex;
    double            realTimeForBestSolution {};
    OptimisationType  optimType;
    bool              updateBound {};
    Problem          &problem;
    double            realTimeStart;

   public:
    explicit Solution(Problem &p, double rt) : problem(p), realTimeStart(rt) { tmp = preserved = nullptr; }


    /**
     * to be called when a new solution will be entered
     */
    void begin(long b) {
        mutex.lock();
        if(!exists() || (optimType == Minimize && b < bound) || (optimType == Maximize && b > bound)) {
            tmp         = new std::vector<int>(problem.nbVariables());
            bound       = b;
            updateBound = true;
        } else
            updateBound = false;
    }


    /**
     * append a literal to the new solution being entered
     */
    void appendTo(int idx, int value) {
        if(updateBound)
            (*tmp)[idx] = value;
    }


    /**
     * to be called once a solution is completely entered
     */
    void end(bool colors) {
        if(updateBound) {
            std::vector<int> *del = preserved;
            // begin critical section
            preserved = tmp;
            // end critical section
            tmp = nullptr;
            delete del;
            realTimeForBestSolution = realTime();
            colorize(termcolor::bright_green, colors);
            std::cout << "o " << bestBound() << " " << (realTimeForBestSolution - realTimeStart) << std::endl;
            resetcolors();
        }
        mutex.unlock();
    }


    long bestBound() const { return (invertBestCost ? -1 : 1) * bound; }

    /**
     * return true is a solution was recorded
     */
    bool exists() { return preserved != nullptr; }

    void cancelSolution() { preserved = nullptr; }

    /**
     * return the latest stable solution
     *
     * even in case of an interrupt, the returned solution is coherent as long
     * as endSolution() is not called in between
     */
    const std::vector<int> &get() const {
        if(!preserved)
            throw std::runtime_error("no solution recorded");

        return *preserved;
    }


    void display() {
        if(exists() == false)
            throw std::runtime_error("The solution does not exist");
        printf("\nv <instantiation type='solution' cost='%ld'>\n", bestBound());
        printf("v <list>");
        for(int i = 0; i < problem.nbVariables(); i++)
            if(problem.variables[i]->_name.rfind("__av", 0) != 0)
                std::cout << problem.variables[i]->_name << " ";

        printf("</list>\n");

        printf("v <values>");
        for(int i = 0; i < problem.nbVariables(); i++) {
            if(problem.variables[i]->_name.rfind("__av", 0) != 0) {
                if((*preserved)[i] == STAR)
                    std::cout << "* ";
                else
                    std::cout << (*preserved)[i] << " ";
            }
        }
        printf("</values>\n");

        printf("v </instantiation>\n\n");
        std::cout << std::flush;
    }
};
}   // namespace Cosoco
#endif   // COSOCO_SOLUTION_H
