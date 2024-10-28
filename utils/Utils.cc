//
// Created by audemard on 16/04/24.
//
#include "Utils.h"

#include <sstream>

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
