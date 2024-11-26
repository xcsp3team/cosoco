/**************************************************************************************[Options.cc]
Copyright (c) 2008-2010, Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
                                                  associated documentation files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
    NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
                                                                                                       OF OR IN CONNECTION WITH
THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
                                                                                                           **************************************************************************************************/

#include "utils/Options.h"

#include <iomanip>
#include <iostream>
#include <set>

#include "mtl/Sort.h"
#include "solver/utils/Options.h"

using namespace Cosoco;


void Cosoco::parseOptions(int& argc, char** argv) {
    std::string str = std::string(argv[1]);
    if(str == "--help")
        printUsageAndExit();
    int i;
    for(i = 2; i < argc; i++) {
        std::string str = std::string(argv[i]);
        if(str == "--help")
            printUsageAndExit();

        str      = str.substr(1, str.size());
        auto pos = str.find('=');
        if(pos == std::string::npos) {
            std::cout << "miss = in option" << str << "\n";
            exit(1);
        }
        std::string o = str.substr(0, pos);
        std::string v = str.substr(pos + 1, str.size());

        // string options
        if(options::stringOptions.find(o) != options::stringOptions.end()) {
            options::stringOptions[o].value = v;
            continue;
        }
        // bool option
        if(options::boolOptions.find(o) != options::boolOptions.end()) {
            if(v != "1" && v != "0") {
                std::cout << "value for bool option is 0 or 1\n";
                exit(1);
            }
            options::boolOptions[o].value = v == "1";
            continue;
        }

        // int option
        if(options::intOptions.find(o) != options::intOptions.end()) {
            int newvalue;
            try {
                newvalue = std::stoi(v);
            } catch(std::invalid_argument const& ex) {
                std::cout << "option " << o << " needs an integer " << v << "is passed\n";
                exit(1);
            }
            if(newvalue < options::intOptions[o].min || newvalue > options::intOptions[o].max) {
                std::cout << "option " << o << " must be  in range " << options::intOptions[o].min << " .. "
                          << options::intOptions[o].max << "\n";
                exit(1);
            }
            options::intOptions[o].value = newvalue;
            continue;
        }
        // int option
        if(options::doubleOptions.find(o) != options::doubleOptions.end()) {
            double newvalue;
            try {
                newvalue = std::stod(v);
            } catch(std::invalid_argument const& ex) {
                std::cout << "option " << o << " needs a double " << v << "is passed\n";
                exit(1);
            }
            if(newvalue < options::doubleOptions[o].min || newvalue > options::doubleOptions[o].max) {
                std::cout << "option " << o << " must be  in range " << options::doubleOptions[o].min << " .. "
                          << options::doubleOptions[o].max << "\n";
                exit(1);
            }
            options::doubleOptions[o].value = newvalue;
            continue;
        }

        std::cout << "unknown option " << o << "\n";
        exit(1);
    }
}
void Cosoco::printUsageAndExit() {
    std::set<std::string> categories;

    for(auto const& it : options::doubleOptions) categories.insert(it.second.category);
    for(auto const& it : options::stringOptions) categories.insert(it.second.category);
    for(auto const& it : options::intOptions) categories.insert(it.second.category);
    for(auto const& it : options::boolOptions) categories.insert(it.second.category);

    for(auto const& cat : categories) {
        std::cout << "\n\n" << cat << " options\n";
        for(auto const& it : options::doubleOptions)
            if(it.second.category == cat)
                std::cout << std::left << std::setw(15) << it.first << std::left << std::setw(15) << std::setfill(' ')
                          << " = <double>   : " << it.second.verbose << " (default: " << it.second.value << ")\n";

        for(auto const& it : options::intOptions)
            if(it.second.category == cat)
                std::cout << std::left << std::setw(15) << it.first << std::left << std::setw(15) << std::setfill(' ')
                          << " = <int>      : " << it.second.verbose << " (default: " << it.second.value << ")\n";

        for(auto const& it : options::stringOptions)
            if(it.second.category == cat)
                std::cout << std::left << std::setw(15) << it.first << std::left << std::setw(15) << std::setfill(' ')
                          << " = <string>   : " << it.second.verbose << " (default: " << it.second.value << ")\n";

        for(auto const& it : options::boolOptions)
            if(it.second.category == cat)
                std::cout << std::left << std::setw(15) << it.first << std::left << std::setw(15) << std::setfill(' ')
                          << " = <0/1>      : " << it.second.verbose << " (default: " << it.second.value << ")\n";
    }
    std::cout << "\n";
    exit(1);
}