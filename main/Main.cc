#include <HeuristicValFirst.h>
#include <HeuristicValLast.h>
#include <optimizer/Optimizer.h>
#include <sys/resource.h>

#include <csignal>
#include <vector>

#include "PortofolioSolver.h"
#include "XCSP3CoreParser.h"
#include "solver/Solver.h"
#include "utils/CosocoCallbacks.h"
#include "utils/Options.h"
#include "utils/System.h"


using namespace Cosoco;
using namespace XCSP3Core;


AbstractSolver       *solver;
vec<AbstractSolver *> solvers;

bool   optimize = false;
double realTimeStart;

string version("2.4");

void displayProblemStatistics(Problem *solvingProblem, double initial_time);

void limitRessourcesUsage(int cpu_lim, int mem_lim);


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
    options::createOptions();
    if(argc == 0) {
        std::cout << "c USAGE: %s [options] <input-file>\n\n  where input may be either in plain or gzipped XCSP3.\n";
        exit(1);
    }

    try {
        printf("c\nc This is cosoco %s  \nc\n", version.c_str());

        // --------------------------- COMMAND LINE ----------------------------------------
        printf("c command line: ");
        for(int i = 0; i < argc; i++) printf("%s ", argv[i]);
        printf("\n");

        parseOptions(argc, argv);
        if((options::intOptions["nbsols"].value > 1 || options::intOptions["nbsols"].value == 0) &&
           options::boolOptions["nogoods"].value == false) {
            cout << "c count solutions without nogoods is impossible" << endl;
            exit(1);
        }

        if(options::intOptions["dsp"].value == 1) {
            execlp("python3", "python3", "structure.py", argv[1], NULL);
            exit(1);
        }
        if(options::intOptions["dsp"].value == 2) {
            execlp("more", "more", argv[1], NULL);
            exit(1);
        }

        limitRessourcesUsage(options::intOptions["cpu_lim"].value, options::intOptions["mem_lim"].value);
        int nbcores = options::intOptions["nbcores"].value;

        // --------------------------- PARSING ----------------------------------------
        vec<CosocoCallbacks> callbacks;
        callbacks.growTo(nbcores);
        vec<Problem *> solvingProblems;

        double initial_time = cpuTime();
        double real_time    = realTime();

        int mem_lim = options::intOptions["mem_lim"].value;
        int mem_used;
        try {
            XCSP3CoreParser parser(&callbacks[0]);
            parser.parse(argv[1]);   // parse the input file
            solvingProblems.push(callbacks[0].problem);
            optimize = callbacks[0].optimizationProblem;
            mem_used = (int)(memUsed() * 1.2);   // Suppose 20% for solver
            if(nbcores > 1 && mem_lim > 0) {
                if(nbcores > mem_lim / mem_used) {
                    nbcores = mem_lim / mem_used;
                    std::cout << "c\nc WARNING: memory for one problem: " << mem_used << ". Memory allowed: " << mem_lim << "\n";
                    std::cout << "c WARNING: limit the number of cores to " << nbcores << " due to memory limit\nc\n";
                }
            }


            for(int i = 1; i < nbcores; i++) {
                XCSP3CoreParser parser2(&callbacks[i]);
                parser2.parse(argv[1]);   // parse the input file
                solvingProblems.push(callbacks[i].problem);
                optimize = callbacks[i].optimizationProblem;
            }
        } catch(exception &e) {
            cout.flush();
            cout << "c " << e.what() << endl;
            if(strstr(e.what(), "not yet")) {
                colorize(termcolor::bright_green, options::boolOptions["colors"].value);
                cout << "s UNSUPPORTED" << endl;
                resetcolors();
            }
            exit(1);
        }


        // if(nbcores > 1)
        //     options.intOptions["verb"].value = 0;

        // --------------------------- INIT SOLVERS ----------------------------------------
        solvers.growTo(nbcores);

        auto *solution = new Solution(*solvingProblems[0], real_time);
        for(int core = 0; core < nbcores; core++) {
            auto *S = new Solver(*solvingProblems[core]);

            S->core = core;
            S->seed = S->seed * (core + 1);
            if(optimize) {
                auto *optimizer           = new Optimizer(*solvingProblems[core]);
                S->displaySolution        = false;
                optimizer->invertBestCost = callbacks[0].invertOptimization;
                optimizer->setSolver(S, solution);


                optimizer->core = core;
                // if(pg && warmStart == nullptr)
                //     optimizer->addProgressSaving();
                solvers[core] = optimizer;
            } else
                solvers[core] = S;

            solvers[core]->setVerbosity(options::intOptions["verb"].value);
        }

        if(options::intOptions["verb"].value >= 2)
            solvingProblems[0]->display();


        displayProblemStatistics(solvingProblems[0], initial_time);


        if(nbcores == 1)
            solver = solvers[0];
        else {
            // PortofolioSolver *ps = new PortofolioSolver();
            ParallelSolver *ps;
            ps = new PortofolioSolver(*solvingProblems[0], optimize);
            printf("c Parallel mode: Portfolio\n");
            ps->setSolvers(solvers);
            solver = ps;
        }


        // --------------------------- SOLVE ----------------------------------------

        int returnCode = solver->solve();
        std::cout << returnCode << std::endl;
        colorize(termcolor::bright_green, options::boolOptions["colors"].value);
        printf(returnCode == R_OPT     ? "s OPTIMUM FOUND\n"
               : returnCode == R_UNSAT ? "s UNSATISFIABLE\n"
               : returnCode == R_SAT   ? "s SATISFIABLE\n"
                                       : "s UNKNOWN\n");
        if(nbcores == 1 && options::boolOptions["model"].value == false && solver->nbSolutions >= 1)
            printf("d N_SOLUTIONS %d\n\n", solvers[0]->nbSolutions);
        resetcolors();
        printStats(solvers[0]);
        printf("\n");

        if(returnCode != R_UNSAT && returnCode != R_UNKNOWN && options::boolOptions["model"].value && solver->hasSolution())
            solver->displayCurrentSolution();


        std::cout << std::flush;
    } catch(OutOfMemoryException &) {
        printf("c =========================================================================================================\n");
        colorize(termcolor::bright_green, options::boolOptions["colors"].value);
        printf("s UNKNOWN\n");
        resetcolors();
        exit(0);
    }
}


void displayProblemStatistics(Problem *solvingProblem, double initial_time) {
    printf("c ========================================[ Problem Statistics ]===========================================\n");
    printf("c |                                                                                                       \n");

    double parsed_time = cpuTime();

    printf("c |  Parse time        : %12.2f s \n", parsed_time - initial_time);
    printf("c |\n");

    printf("c |               ");
    colorize(termcolor::blue, options::boolOptions["colors"].value);
    printf("Variables:");
    resetcolors();
    printf(" %d (original: %d -- auxiliary: %d)\n", solvingProblem->nbVariables(), solvingProblem->nbOriginalVars,
           solvingProblem->nbVariables() - solvingProblem->nbOriginalVars);
    printf("c |            ");
    colorize(termcolor::blue, options::boolOptions["colors"].value);
    printf("Domain Sizes: ");
    resetcolors();
    printf("%d..%d\n", solvingProblem->minimumDomainSize(), solvingProblem->maximumDomainSize());
    printf("c |            ");
    resetcolors();
    colorize(termcolor::blue, options::boolOptions["colors"].value);
    printf("      Values: ");
    resetcolors();
    printf("%ld\n", solvingProblem->nbValues());
    printf("c |             ");
    colorize(termcolor::blue, options::boolOptions["colors"].value);
    printf("Constraints: ");
    resetcolors();
    printf("%d\n", solvingProblem->nbConstraints());
    printf("c |                   ");
    colorize(termcolor::blue, options::boolOptions["colors"].value);
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
    colorize(termcolor::blue, options::boolOptions["colors"].value);
    printf("Types: ");
    resetcolors();
    printf("\nc |                          ");

    std::map<std::string, int> typeOfConstraints;
    solvingProblem->nbTypeOfConstraints(typeOfConstraints);

    std::set<string> bigConstraints;
    for(Constraint *c : solvingProblem->constraints)
        if(c->scope.size() > 100)
            bigConstraints.insert(c->type);

    for(auto &iter : typeOfConstraints) {
        if(iter.first == "Extension")
            printf("Extension: %d  (nb tuples: %d..%d) -- (shared: %d)\nc |                          ", iter.second,
                   solvingProblem->minimumTuplesInExtension(), solvingProblem->maximumTuplesInExtension(),
                   solvingProblem->nbExtensionsSharded);
        else
            printf("%s: %d\nc |                          ", iter.first.c_str(), iter.second);
    }
    if(bigConstraints.size() > 0) {
        printf("\nc |                   ");
        colorize(termcolor::blue, options::boolOptions["colors"].value);
        printf("Big constraints: ");
        resetcolors();
        for(auto &s : bigConstraints) std::cout << s << " ";
        std::cout << "\n";
    }

    if(optimize) {
        ObjectiveConstraint *objective;
        printf("\n");
        printf("c |               ");
        colorize(termcolor::blue, options::boolOptions["colors"].value);
        printf("Objective: ");
        resetcolors();
        auto *o   = (Optimizer *)solvers[0];
        objective = (o->optimtype == Minimize) ? o->objectiveUB : o->objectiveLB;
        auto *c   = dynamic_cast<Constraint *>(objective);
        printf("%s %s\n", o->optimtype == Minimize || (o->optimtype == Maximize && o->invertBestCost) ? "Minimize" : "Maximize",
               c->type.c_str());
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


void limitRessourcesUsage(int cpu_lim, int mem_lim) {
    // Set limit on CPU-time:
    if(cpu_lim > 0) {
        rlimit rl;
        getrlimit(RLIMIT_CPU, &rl);

        if(rl.rlim_max == RLIM_INFINITY || (rlim_t)cpu_lim < rl.rlim_max) {
            rl.rlim_cur = cpu_lim;
            if(setrlimit(RLIMIT_CPU, &rl) == -1)
                printf("c WARNING! Could not set resource limit: CPU-time.\n");
            else
                printf("c Limit cpu time to %d\n", cpu_lim);
        }
    }

    // Set limit on virtual memory:
    if(mem_lim > 0) {
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


// Note that '_exit()' rather than 'exit()' has to be used. The reason is that 'exit()' calls
// destructors and may cause deadlocks if a malloc/free function happens to be running (these
// functions are guarded by locks for multithreaded use).
static void SIGINT_exit(int signum) {
    printf("\n");
    printf("c *** INTERRUPTED ***\n");

    if(true) {
        if(options::intOptions["verb"].value >= 0) {
            printStats(solvers[0]);
            printf("\n");
            colorize(termcolor::bright_green, options::boolOptions["colors"].value);
            if(solvers[0]->nbSolutions >= 1)
                printf("d N_SOLUTIONS %d\n", solvers[0]->nbSolutions);
            resetcolors();

            if(optimize) {
                if(solvers[0]->hasSolution()) {
                    printf("s SATISFIABLE\n");
                    printf("c Best value found: %ld\n\n", ((Optimizer *)solvers[0])->bestCost());
                    if(options::boolOptions["model"].value)
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
