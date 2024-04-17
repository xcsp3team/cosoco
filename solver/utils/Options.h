//
// Created by audemard on 15/04/24.
//

#include <climits>
#include <map>
#include <string>
#include <utility>
#ifndef COSOCO_OPTIONS_H


namespace Cosoco {


struct str_opt {
    std::string category, verbose, value;
};

struct bool_opt {
    std::string category, verbose;
    bool        value;
};

struct int_opt {
    std::string category, verbose;
    int         value, min, max;
};


struct double_opt {
    std::string category, verbose;
    double      value, min, max;
    double      operator()() const { return value; }
};

class Options {
   public:
    std::map<std::string, str_opt>    stringOptions;
    std::map<std::string, bool_opt>   boolOptions;
    std::map<std::string, int_opt>    intOptions;
    std::map<std::string, double_opt> doubleOptions;

    Options() {
        intOptions["cpu_lim"] = {"MAIN", "Limit on CPU time allowed in seconds", 0, 0, INT_MAX};
        intOptions["mem_lim"] = {"MAIN", "Limit on MEM time allowed in megabytes", 0, 0, INT_MAX};
        intOptions["verb"]    = {"MAIN", "Verbosity level (0=silent, 1=some, 2=more, 3=full, 4=fullfull).", 1, 0, 4};
        intOptions["nbsols"]  = {"MAIN", "Number of solutions to find", 1, 0, INT_MAX};
        boolOptions["model"]  = {"MAIN", "Display models", 0};
        boolOptions["colors"] = {"MAIN", "Add colors to output", 1};

        boolOptions["nogoods"]     = {"SEARCH", "Learn nogoods from restarts", true};
        intOptions["lc"]           = {"SEARCH", "Last Conflict reasoning (0 to disable)", 1, 0, 100};
        boolOptions["stick"]       = {"SEARCH", "Sticking Value on heuristic val", false};
        boolOptions["restarts"]    = {"SEARCH", "Enable restarts", true};
        stringOptions["val"]       = {"SEARCH", "Heuristic for values (first, last, random, robin, occs, asgs, pool)", "first"};
        stringOptions["robin"]     = {"SEARCH", "sequence for robin (F (first), L(last), R(random), O(occs), A(asgs))", "FLR"};
        stringOptions["var"]       = {"SEARCH", "Heuristic for variables (wdeg, cacd, pick)", "wdeg"};
        intOptions["limitac3card"] = {"SEARCH", "Max size to perform AC3 on intensional constraints", 1000, 0, INT_MAX};
        boolOptions["annotations"] = {"SEARCH", "Enable annotations (if any)", true};
        stringOptions["warmstart"] = {"SEARCH", "add a FILE that contains a list of values used as heuristic val", ""};
        boolOptions["bs"]          = {"SEARCH", "Enable progress saving (only after a new solution)", true};


        intOptions["i2e"] = {"PARSE", "Transform intension to extension. Max size of cartesian product (0 -> disable it)", 100000,
                             0, INT_MAX};
        stringOptions["removeclasses"] = {"PARSE", "Remove special classes when parsing (symmetryBreaking,redundant...)", ""};
        boolOptions["decompose"]       = {"PARSE", "decompose intension using reification.", true};
    }
};


}   // namespace Cosoco
#define COSOCO_OPTIONS_H

#endif   // COSOCO_OPTIONS_H
