#pragma once
#ifndef COSOCO_MDD_H
#define COSOCO_MDD_H

#ifdef USE_XCSP3

#include <core/Variable.h>
#include <mtl/Vec.h>

#include <string>

#include "XCSP3Constraint.h"
#define falseNode nullptr

namespace Cosoco {
class MDDNode {
   public:
    int            id;
    int            level;
    std::string    name;
    vec<MDDNode *> childs;
    vec<int>       directAccessToChilds;


    MDDNode(std::string n, int _id, int lvl, int maxNbChilds);
    int  nbChilds() const;
    bool isRoot() const;
    void display();
    void addChild(int idv, MDDNode *target);
};


class MDD {
   protected:
    MDD() { }

   public:
    MDDNode *root;
    MDDNode *trueNode;
    int      nbNodes;
    u_char  *memoryNodes;   // Stores nodes in a contiguous memory zone


    MDD(vec<XCSP3Core::XTransition *> &transitions, vec<Variable *> &scope);
    virtual ~MDD();

    static MDD *buildFromAutomata(std::string name, vec<Variable *> &scope, string start, std::vector<string> &finals,
                                  vec<XCSP3Core::XTransition *> &transitions);
};
}   // namespace Cosoco

#endif /* USE_XCSP3 */

#endif   // COSOCO_MDD_H
