#include <optimizer/Optimizer.h>
#include <sys/resource.h>

#include <csignal>
#include <cstring>
#include <sstream>
#include <vector>

#include "HeuristicValASGS.h"
#include "HeuristicValOccs.h"
#include "HeuristicVarCACD.h"
#include "PickOnDom.h"
#include "XCSP3CoreParser.h"
#include "solver/Solver.h"
#include "solver/heuristics/values/HeuristicValLast.h"
#include "solver/heuristics/values/HeuristicValRandom.h"
#include "solver/heuristics/values/HeuristicValRoundRobin.h"
#include "solver/heuristics/values/PoolOfHeuristicsValues.h"
#include "utils/CosocoCallbacks.h"
#include "utils/Options.h"
#include "utils/System.h"


using namespace Cosoco;
using namespace XCSP3Core;


AbstractSolver       *solver;
vec<AbstractSolver *> solvers;

bool   optimize = false;
double realTimeStart;

string version("2.2");
// --------------------------- OPTIONS ----------------------------------------

IntOption  verb("MAIN", "verb", "Verbosity level (0=silent, 1=some, 2=more, 3=full, 4=fullfull).", 1, IntRange(0, 4));
IntOption  cpu_lim("MAIN", "cpu-lim", "Limit on CPU time allowed in seconds.\n", INT32_MAX, IntRange(0, INT32_MAX));
IntOption  mem_lim("MAIN", "mem-lim", "Limit on memory usage in megabytes.\n", INT32_MAX, IntRange(0, INT32_MAX));
IntOption  nbSolutions("MAIN", "nbsols", "Number of solutions to find", 1, IntRange(0, INT32_MAX));
BoolOption model("MAIN", "model", "Display models", 0);
BoolOption colors("MAIN", "colors", "Add colors to output", 1);


BoolOption nogoods("SEARCH", "nogoods", "Learn nogoods from restarts", 0);
IntOption  lastConflict("SEARCH", "lc", "Last Conflict reasoning (0 to disable)", 1);
BoolOption sticking("SEARCH", "stick", "Sticking Value on heuristic val", 0);
// BoolOption optimize("SEARCH", "cop", "Run optimizer (needs an objective)", 0);
BoolOption   orestarts("SEARCH", "restarts", "Enable restarts", 1);
StringOption hv("SEARCH", "val", "Heuristic for values (first, last, random, robin, occs, asgs, pool)", "first");
StringOption robin("SEARCH", "robin",
                   "The order of robin (F (first), L(last), R(random), O(occs), A(asgs)), usefull only if val=robin", "FLR");
StringOption hvr("SEARCH", "var", "Heuristic for values (wdeg, cacd)", "wdeg");

BoolOption   annotations("SEARCH", "annotations", "Enable annotations (if any)", true);
StringOption warmStart("warmstart", "warmstart", "add a FILE that contains a list of values used as heuristic val");
BoolOption   pg("OPT", "bs", "Enable progress saving (only after a new solution)", true);
StringOption removeClasses("PARSE", "removeclasses", "Remove special classes when parsing (symmetryBreaking,redundant...)");
IntOption    i2e("PARSE", "i2e", "Transform intension to extension. Max size of cartesian product (0 -> disable it", 100000,
                 IntRange(0, INT32_MAX));

// --------------------------- OPTIONS ----------------------------------------


void displayProblemStatistics(Problem *solvingProblem, double initial_time);

void limitRessourcesUsage();

std::vector<std::string> &split1(const std::string &s, char delim, std::vector<std::string> &elems);

std::vector<std::string> split1(const std::string &s, char delim);

void printStats(AbstractSolver *solver);

static void SIGINT_exit(int signum);


// Terminate by notifying the solver and back out gracefully. This is mainly to have a test-case
// for this feature of the Solver as it may take longer than an immediate call to '_exit()'.
static void SIGINT_interrupt(int signum) {
    std::cout << "\nc Kill process\n";
    SIGINT_exit(0);
}


//=================================================================================================
// Main:

int main(int argc, char **argv) {
    realTimeStart = realTime();
    int nbcores   = 1;

    try {
        printf("c\nc This is cosoco %s  \nc\n", version.c_str());

        setUsageHelp("c USAGE: %s [options] <input-file>\n\n  where input may be either in plain or gzipped XCSP3.\n");

        // --------------------------- COMMAND LINE ----------------------------------------
        printf("c command line: ");
        for(int i = 0; i < argc; i++) printf("%s ", argv[i]);
        printf("\n");

        parseOptions(argc, argv, true);
        std::cout << orestarts << endl;
        if((nbSolutions > 1 || nbSolutions == 0) && orestarts) {
            cout << "c count solutions and restarts is impossible" << endl;
            exit(1);
        }

        if((nbSolutions > 1 || nbSolutions == 0) && nbcores > 1) {
            cout << "c count solutions and multicore usage is impossible" << endl;
            exit(1);
        }

        limitRessourcesUsage();

        // --------------------------- PARSING ----------------------------------------
        CosocoCallbacks cb(nbcores, i2e);
        vec<Problem *>  solvingProblems;

        double initial_time = cpuTime();

        if(removeClasses != nullptr) {
            std::vector<std::string> classes = split1(std::string(removeClasses), ',');
            for(const std::string &c : classes) cb.addClassToDiscard(c);
        }

        try {
            XCSP3CoreParser parser(&cb);
            parser.parse(argv[1]);   // parse the input file
            cb.problems.copyTo(solvingProblems);
            optimize = cb.optimizationProblem;
        } catch(exception &e) {
            cout.flush();
            cout << "c " << e.what() << endl;
            if(strstr(e.what(), "not yet")) {
                colorize(termcolor::bright_green, colors);
                cout << "s UNSUPPORTED" << endl;
                resetcolors();
            }
            exit(1);
        }


        if(nbcores > 1)
            verb = 0;

        // --------------------------- INIT SOLVERS ----------------------------------------
        solvers.growTo(nbcores);
        auto *solution = new Solution(*solvingProblems[0]);
        for(int core = 0; core < nbcores; core++) {
            auto *S                     = new Solver(*solvingProblems[core]);
            S->core                     = core;
            S->seed                     = S->seed * (core + 1);
            S->intension2extensionLimit = i2e;
            S->colors                   = colors;
            if(strcmp(hv, "first") != 0 && strcmp(hv, "last") != 0 && strcmp(hv, "rand") != 0 && strcmp(hv, "robin") != 0 &&
               strcmp(hv, "occs") != 0 && strcmp(hv, "asgs") != 0 && strcmp(hv, "pool") != 0) {
                fprintf(stderr, "  --help        Print help message.\n");
                exit(1);
            }
            string tmp(robin);
            if(strcmp(hv, "rand") == 0)
                S->heuristicVal = new HeuristicValRandom(*S);
            if(strcmp(hv, "last") == 0)
                S->heuristicVal = new HeuristicValLast(*S);
            if(strcmp(hv, "robin") == 0)
                S->heuristicVal = new HeuristicValRoundRobin(*S, tmp);
            if(strcmp(hv, "occs") == 0)
                S->heuristicVal = new HeuristicValOccs(*S);
            if(strcmp(hv, "asgs") == 0)
                S->heuristicVal = new HeuristicValASGS(*S);
            if(strcmp(hv, "pool") == 0)
                S->heuristicVal = new PoolOfHeuristicsValues(*S);

            if(strcmp(hvr, "wdeg") != 0 && strcmp(hvr, "cacd") != 0 && strcmp(hvr, "pick") != 0) {
                fprintf(stderr, "  --help        Print help message.\n");
                exit(1);
            }
            if(strcmp(hvr, "cacd") == 0)
                S->heuristicVar = new HeuristicVarCACD(*S);
            if(strcmp(hvr, "pick") == 0)
                S->heuristicVar = new PickOnDom(*S);

            if(warmStart != nullptr) {
                std::ifstream warmFile(warmStart);
                vec<int>      values;
                for(std::string line; std::getline(warmFile, line);) {
                    std::vector<std::string> stringValues = split1(line, ' ');
                    for(const std::string &v : stringValues) {
                        if(v.find("x") != std::string::npos) {
                            std::vector<std::string> compact = split1(v, 'x');
                            int tmp = compact[0].find("*") != std::string::npos ? STAR : std::stoi(compact[0]);
                            for(int i = 0; i < std::stoi(compact[1]); i++) values.push(tmp);
                        } else {
                            if(v.find("*") != std::string::npos) {
                                values.push(STAR);
                            } else {
                                values.push(std::stoi(v));
                            }
                        }
                    }
                }
                S->warmStart    = true;
                S->heuristicVal = new ForceIdvs(*S, S->heuristicVal, false, &values);
            }

            if(lastConflict)
                S->addLastConflictReasoning(lastConflict);
            if(sticking)
                S->addStickingValue();
            if(orestarts)
                S->addRestart();
            if(nogoods)
                S->addNoGoodsFromRestarts();

            S->nbWishedSolutions = nbSolutions;
            if(annotations && cb.decisionVariables[core].size() != 0)
                S->setDecisionVariables(cb.decisionVariables[core]);
            else {
                if(0 && cb.nbInitialsVariables != solvingProblems[0]->nbVariables()) {
                    vec<Variable *> tmp;
                    for(int i = 0; i < cb.nbInitialsVariables; i++) tmp.push(solvingProblems[0]->variables[i]);
                    S->setDecisionVariables(tmp);
                    printf("%d\n", tmp.size());
                    if(core == 0)
                        S->verbose.log(NORMAL, "c Limit decision variables to initial ones (%d vars)\n", cb.nbInitialsVariables);
                }
            }

            if(optimize) {
                auto *optimizer           = new Optimizer(*solvingProblems[core]);
                optimizer->invertBestCost = cb.invertOptimization;
                optimizer->setSolver(S, solution);
                optimizer->core   = core;
                optimizer->colors = colors;
                if(pg && warmStart == nullptr)
                    optimizer->addProgressSaving();
                solvers[core] = optimizer;
            } else
                solvers[core] = S;

            solvers[core]->displayModels = model;
            solvers[core]->setVerbosity(verb);
        }

        if(verb >= 2)
            solvingProblems[0]->display();


        displayProblemStatistics(solvingProblems[0], initial_time);


        if(nbcores == 1)
            solver = solvers[0];
        else {
            assert(false);
        }

        // --------------------------- SOLVE ----------------------------------------

        int returnCode = solver->solve();
        colorize(termcolor::bright_green, colors);
        printf(returnCode == R_OPT     ? "s OPTIMUM FOUND\n"
               : returnCode == R_UNSAT ? "s UNSATISFIABLE\n"
               : returnCode == R_SAT   ? "s SATISFIABLE\n"
                                       : "s UNKNOWN\n");
        if(nbcores == 1 && model == false && solver->nbSolutions >= 1)
            printf("d N_SOLUTIONS %d\n\n", solvers[0]->nbSolutions);
        resetcolors();
        printStats(solvers[0]);
        printf("\n");

        if(returnCode != R_UNSAT && returnCode != R_UNKNOWN && model && solver->hasSolution())
            solver->displayCurrentSolution();


        std::cout << std::flush;

    } catch(OutOfMemoryException &) {
        printf("c =========================================================================================================\n");
        colorize(termcolor::bright_green, colors);
        printf("s UNKNOWN\n");
        resetcolors();
        exit(0);
    }
}


void displayProblemStatistics(Problem *solvingProblem, double initial_time) {
    if(optimize)
        printf("c enable optimization\n");

    if(verb > 0) {
        printf("c ========================================[ Problem Statistics ]===========================================\n");
        printf("c |                                                                                                       \n");
    }


    double parsed_time = cpuTime();
    if(verb > 0) {
        printf("c |  Parse time        : %12.2f s \n", parsed_time - initial_time);
        printf("c |\n");

        printf("c |               ");
        colorize(termcolor::blue, colors);
        printf("Variables:");
        resetcolors();
        printf(" %d\n", solvingProblem->nbVariables());
        printf("c |            ");
        colorize(termcolor::blue, colors);
        printf("Domain Sizes: ");
        resetcolors();
        printf("%d..%d\n", solvingProblem->minimumDomainSize(), solvingProblem->maximumDomainSize());
        printf("c |\n");
        printf("c |             ");
        colorize(termcolor::blue, colors);
        printf("Constraints: ");
        resetcolors();
        printf("%d\n", solvingProblem->nbConstraints());
        printf("c |                   ");
        colorize(termcolor::blue, colors);
        printf("Arity: ");
        resetcolors();
        printf("%d..%d", solvingProblem->minimumArity(), solvingProblem->maximumArity());
        int nb;
        if((nb = solvingProblem->nbConstraintsOfSize(1)) > 0)
            printf("  -- Unary: %d", nb);
        if((nb = solvingProblem->nbConstraintsOfSize(2)) > 0)
            printf("  -- Binary: %d", nb);
        if((nb = solvingProblem->nbConstraintsOfSize(3)) > 0)
            printf("  -- Ternary: %d", nb);

        printf("\nc | \n");
        printf("c |                   ");
        colorize(termcolor::blue, colors);
        printf("Types: ");
        resetcolors();
        printf("\nc |                          ");

        std::map<std::string, int> typeOfConstraints;
        solvingProblem->nbTypeOfConstraints(typeOfConstraints);
        for(auto &iter : typeOfConstraints) {
            if(iter.first == "Extension")
                printf("Extension: %d  (nb tuples: %d..%d) -- (shared: %d)\nc |                          ", iter.second,
                       solvingProblem->minimumTuplesInExtension(), solvingProblem->maximumTuplesInExtension(),
                       Extension::nbShared);
            else
                printf("%s: %d\nc |                          ", iter.first.c_str(), iter.second);
        }

        if(optimize) {
            ObjectiveConstraint *objective;
            printf("\n");
            printf("c |               ");
            colorize(termcolor::blue, colors);
            printf("Objective: ");
            resetcolors();
            auto *o   = (Optimizer *)solvers[0];
            objective = (o->optimtype == Minimize) ? o->objectiveUB : o->objectiveLB;
            auto *c   = dynamic_cast<Constraint *>(objective);
            printf("%s %s\n",
                   o->optimtype == Minimize || (o->optimtype == Maximize && o->invertBestCost) ? "Minimize" : "Maximize",
                   c->type.c_str());
        }
    }
    printf("\n");
    printf("c =========================================================================================================\n");
}


void printStats(AbstractSolver *solver) {
    double cpu_time = cpuTime();
    double mem_used = memUsedPeak();

    solver->printFinalStats();

    if(mem_used != 0)
        printf("c Memory used           : %.2f MB\n", mem_used);
    auto *optimizer = dynamic_cast<Optimizer *>(solver);
    if(optimizer != nullptr)
        printf("c Best bound time (wc)  : %g s\n", optimizer->realTimeForBestSolution() - realTimeStart);
    printf("c CPU time              : %g s\n", cpu_time);
    printf("c real time             : %g s\n", realTime() - realTimeStart);
}


void limitRessourcesUsage() {
    // Set limit on CPU-time:
    if(cpu_lim != INT32_MAX) {
        rlimit rl;
        getrlimit(RLIMIT_CPU, &rl);

        if(rl.rlim_max == RLIM_INFINITY || (rlim_t)cpu_lim < rl.rlim_max) {
            rl.rlim_cur = cpu_lim;
            if(setrlimit(RLIMIT_CPU, &rl) == -1)
                printf("c WARNING! Could not set resource limit: CPU-time.\n");
            else
                printf("c Limit cpu time to %d\n", cpu_lim.operator int());
        }
    }

    // Set limit on virtual memory:
    if(mem_lim != INT32_MAX) {
        rlim_t new_mem_lim = (rlim_t)mem_lim * 1024 * 1024;
        rlimit rl;
        getrlimit(RLIMIT_AS, &rl);
        if(rl.rlim_max == RLIM_INFINITY || new_mem_lim < rl.rlim_max) {
            rl.rlim_cur = new_mem_lim;
            if(setrlimit(RLIMIT_AS, &rl) == -1)
                printf("c WARNING! Could not set resource limit: Virtual memory.\n");
        }
    }

    // Change to signal-handlers that will only notify the solver and allow it to terminate
    // voluntarily:
    signal(SIGINT, SIGINT_interrupt);
    signal(SIGXCPU, SIGINT_interrupt);
    signal(SIGTERM, SIGINT_interrupt);
    signal(SIGABRT, SIGINT_interrupt);
}


std::vector<std::string> &split1(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string       item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split1(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split1(s, delim, elems);
    return elems;
}


// Note that '_exit()' rather than 'exit()' has to be used. The reason is that 'exit()' calls
// destructors and may cause deadlocks if a malloc/free function happens to be running (these
// functions are guarded by locks for multithreaded use).
static void SIGINT_exit(int signum) {
    printf("\n");
    printf("c *** INTERRUPTED ***\n");

    if(true /*nbcores == 1*/) {
        if(verb >= 0) {
            printStats(solvers[0]);
            printf("\n");
            colorize(termcolor::bright_green, colors);
            if(solvers[0]->nbSolutions >= 1)
                printf("d N_SOLUTIONS %d\n", solvers[0]->nbSolutions);
            if(optimize) {
                if(solvers[0]->hasSolution()) {
                    printf("s SATISFIABLE\n");
                    printf("c Best value found: %ld\n\n", ((Optimizer *)solvers[0])->bestCost());
                    if(solvers[0]->displayModels)
                        solvers[0]->displayCurrentSolution();
                } else
                    printf("c No Bound found!\n\ns UNKNOWN\n");

            } else {
                if(solvers[0]->nbSolutions > 0)
                    printf("s SATISFIABLE\n");
                else
                    printf("s UNKNOWN\n");
            }
            resetcolors();
        }
        std::cout << std::flush;

        exit(1);
    }
}
