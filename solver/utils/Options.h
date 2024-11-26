//
// Created by audemard on 15/04/24.
//

#include <climits>
#include <map>
#include <string>
#include <utility>
#ifndef COSOCO_OPTIONS_H


namespace Cosoco {
namespace options {

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

extern std::map<std::string, str_opt>    stringOptions;
extern std::map<std::string, bool_opt>   boolOptions;
extern std::map<std::string, int_opt>    intOptions;
extern std::map<std::string, double_opt> doubleOptions;

void createOptions();

}   // namespace options
}   // namespace Cosoco
#define COSOCO_OPTIONS_H

#endif   // COSOCO_OPTIONS_H