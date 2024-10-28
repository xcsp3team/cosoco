//
// Created by audemard on 10/11/2015.
//
#pragma once

#ifndef COSOCO_UTILS_H
#define COSOCO_UTILS_H

#include <string>
#include <vector>

template <class T>
const T &max(const T &a, const T &b) {
    return (a < b) ? b : a;   // or: return comp(a,b)?b:a; for version (2)
}


// Returns a random float 0 <= x < 1. Seed must never be 0.
inline double drand(double &seed) {
    seed *= 1389796;
    int q = (int)(seed / 2147483647);
    seed -= (double)q * 2147483647;
    return seed / 2147483647;
}


// Returns a random integer 0 <= x < size. Seed must never be 0.
inline int irand(double &seed, int size) { return (int)(drand(seed) * size); }

std::vector<std::string> &split1(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string>  split1(const std::string &s, char delim);

#ifdef USE_XCSP3
#include "XCSP3TreeNode.h"
inline bool nodeContainsVar(XCSP3Core::Node *n, std::string name) {
    if(n->type == XCSP3Core::OVAR) {
        auto *nv = dynamic_cast<XCSP3Core::NodeVariable *>(n);
        if(nv->var == name)
            return true;
    }
    if(n->type == XCSP3Core::ODECIMAL)
        return false;
    for(XCSP3Core::Node *tmp : n->parameters)
        if(nodeContainsVar(tmp, name))
            return true;
    return false;
}

#endif /* USE_XCSP3 */


#endif   // COSOCO_UTILS_H
