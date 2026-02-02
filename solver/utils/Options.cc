#include "Options.h"

namespace Cosoco {
namespace options {
std::map<std::string, Cosoco::options::str_opt>    stringOptions;
std::map<std::string, Cosoco::options::bool_opt>   boolOptions;
std::map<std::string, Cosoco::options::int_opt>    intOptions;
std::map<std::string, Cosoco::options::double_opt> doubleOptions;
std::map<std::string, int>                         intConstants;

void createOptions() {
    intOptions["cpu_lim"]         = {"MAIN", "Limit on CPU time allowed in seconds", 0, 0, INT_MAX};
    intOptions["mem_lim"]         = {"MAIN", "Limit on MEM time allowed in megabytes", 0, 0, INT_MAX};
    intOptions["verb"]            = {"MAIN", "Verbosity level (0=silent, 1=some, 2=more, 3=full, 4=fullfull).", 1, 0, 4};
    intOptions["nbsols"]          = {"MAIN", "Number of solutions to find", 1, 0, INT_MAX};
    intOptions["model"]           = {"MAIN", "Display models (0: none, 1: minimum verbosity, 2: full verbosity", 0, 0, 2};
    boolOptions["colors"]         = {"MAIN", "Add colors to output", true};
    boolOptions["profile"]        = {"MAIN", "Profile the solver", false};
    boolOptions["checksolutions"] = {"MAIN", "Check all solutions", true};
    intOptions["dsp"]             = {
        "MAIN", "1: Display the structure of the problem and exit (using python script). 2: display the xml file", 0, 0, 2};
    intOptions["nbcores"]  = {"MAIN", "Number of threads to use", 1, 1, INT_MAX};
    boolOptions["options"] = {"MAIN", "Display Selected options", 0};

    boolOptions["nogoods"]     = {"SEARCH", "Learn nogoods from restarts", true};
    intOptions["lc"]           = {"SEARCH", "Last Conflict reasoning (0 to disable)", 1, 0, 100};
    boolOptions["stick"]       = {"SEARCH", "Sticking Value on heuristic val", false};
    stringOptions["restarts"]  = {"SEARCH", "Restarts: no (no restarts), luby, geo, io (inner outer)", "io"};
    stringOptions["val"]       = {"SEARCH", "Heuristic for values (first, last, random, robin, occs, asgs, pool, max)", "first"};
    stringOptions["robin"]     = {"SEARCH", "sequence for robin (F (first), L(last), R(random), O(occs), A(asgs))", "FLR"};
    stringOptions["var"]       = {"SEARCH", "Heuristic for variables (wdeg, cacd, pick, frba, robin)", "robin"};
    boolOptions["lazyvar"]     = {"SEARCH", "Compute lazily var heuristics", 0};
    intOptions["limitac3card"] = {"SEARCH", "Max size to perform AC3 on intensional constraints", 1000, 0, INT_MAX};
    boolOptions["annotations"] = {"SEARCH", "Enable annotations (if any)", true};
    stringOptions["warmstart"] = {"SEARCH", "add a FILE that contains a list of values used as heuristic val", ""};
    intOptions["disablesingleton"] = {"SEARCH", "disable singleton variables if nb vars >=", 750};
    boolOptions["bs"]              = {"SEARCH", "Enable progress saving (only after a new solution)", true};
    boolOptions["rw"]              = {"SEARCH", "Enable weights reseting for heuristics every 30 restarts", false};
    boolOptions["ct"]              = {"CONSTRAINTS", "Enable Compact Table", true};
    intOptions["postponesize"]     = {"CONSTRAINTS", "The size of postponed constraints (0=no postponed constraints)", 100, 0,
                                      INT_MAX};
    intOptions["i2e"] = {"PARSE", "Transform intension to extension. Max size of cartesian product (0 -> disable it)", 100000, 0,
                         INT_MAX};
    stringOptions["removeclasses"] = {"PARSE", "Remove special classes when parsing (symmetryBreaking,redundant...)", ""};
    boolOptions["decompose"]       = {"PARSE", "decompose intension using reification.", true};
    intOptions["bound"]            = {"OPTIMIZER", "set the initial bound to the value", INT_MAX, INT_MIN, INT_MAX};
    intOptions["rr"]               = {"OPTIMIZER", "heuristic Var and Val Robin with Bs deactivated during first runs", 0, 0, 2};

    intConstants["large_bin_extension"] = 1000;
    intConstants["smallNbTuples"]       = 16;   // if tuples.size() < 16 then create STR0 constraint, otherwise STR2/CT
}


}   // namespace options
}   // namespace Cosoco