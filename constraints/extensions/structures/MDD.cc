#include "MDD.h"

#include <map>
#include <new>
#include <set>

#include "MDD.h"
using namespace Cosoco;


//----------------------------------------------
// MDD
//----------------------------------------------

MDD::MDD(vec<XCSP3Core::XTransition *> &transitions, vec<Variable *> &scope) {
    std::map<std::string, MDDNode *> nodes;
    std::set<string>                 possibleRoots, possibleLeaves;
    std::set<std::string>::iterator  it;
    unsigned int                     nb = 0;
    for(XCSP3Core::XTransition *tr : transitions) {
        possibleLeaves.insert(tr->to);
        possibleRoots.insert(tr->from);
    }
    for(XCSP3Core::XTransition *tr : transitions) {
        std::string src = tr->from;
        std::string tgt = tr->to;
        possibleLeaves.erase(src);
        possibleRoots.erase(tgt);
    }

    assert(possibleLeaves.size() == 1);
    assert(possibleRoots.size() == 1);

    std::set<string> nodeNames;
    for(XCSP3Core::XTransition *tr : transitions) {
        nodeNames.insert(tr->from);
        nodeNames.insert(tr->to);
    }
    memoryNodes = new u_char[sizeof(MDDNode) * nodeNames.size()];
    u_char *ptr = memoryNodes;


    root = new(ptr) MDDNode(*possibleRoots.begin(), nb++, 0, scope[0]->domain.maxSize());
    ptr += sizeof(MDDNode);
    nodes[root->name] = root;

    trueNode = new(ptr) MDDNode(*possibleLeaves.begin(), -1, -1, 0);
    ptr += sizeof(MDDNode);
    nodes[trueNode->name] = trueNode;

    for(XCSP3Core::XTransition *tr : transitions) {
        MDDNode *src = nodes[tr->from];
        assert(src != nullptr);   // We suppose the first tuple is the root!
        int      idv = scope[src->level]->domain.toIdv(tr->val);
        MDDNode *tgt = nodes[tr->to];
        if(tgt == nullptr) {
            assert(src->level + 1 < scope.size());
            tgt = new(ptr) MDDNode(tr->to, nb++, src->level + 1, scope[src->level + 1]->domain.maxSize());
            ptr += sizeof(MDDNode);
            nodes[tgt->name] = tgt;
        }
        src->addChild(idv, tgt);
    }
    trueNode->id = nb++;
    assert(nb == nodes.size());
    nbNodes = nodes.size();
}


MDD::~MDD() {
    // TODO
}

//----------------------------------------------
// Build from Automata
//----------------------------------------------


MDD *MDD::buildFromAutomata(std::string name, vec<Variable *> &scope, string start, std::vector<string> &finals,
                            vec<XCSP3Core::XTransition *> &transitions) {
    // Build Map state-> possible state
    std::map<std::string, vec<XCSP3Core::XTransition *> *> nextTransitions;
    for(XCSP3Core::XTransition *t : transitions) {
        if(nextTransitions.find(t->from) == nextTransitions.end())
            nextTransitions[t->from] = new vec<XCSP3Core::XTransition *>();
        nextTransitions[t->from]->push(t);
    }
    for(XCSP3Core::XTransition *t : transitions) {
        if(nextTransitions.find(t->to) == nextTransitions.end())
            nextTransitions[t->to] = new vec<XCSP3Core::XTransition *>();
    }

    int  nb  = 0;
    MDD *tmp = new MDD();

    tmp->root     = new MDDNode(start, nb++, 0, scope[0]->domain.maxSize());
    tmp->trueNode = new MDDNode("leaf", -1, -1, 0);

    std::map<std::string, MDDNode *> prevs, nexts;
    prevs[start] = tmp->root;

    for(int i = 0; i < scope.size(); i++) {
        for(std::map<std::string, MDDNode *>::iterator it = prevs.begin(); it != prevs.end(); ++it) {
            MDDNode *node = it->second;
            for(XCSP3Core::XTransition *tr : *nextTransitions[node->name]) {
                assert(node->name == tr->from);

                int val = tr->val;
                int idv = scope[i]->domain.toIdv(val);
                if(idv != -1) {   // Contain the value
                    std::string nextState = tr->to;
                    if(i == scope.size() - 1) {
                        // node->addChild(idv, finals.contains(nextState) ? tmp->trueNode : falseNode);
                        node->addChild(
                            idv, find(finals.begin(), finals.end(), nextState) != finals.end() /*finals.contains(nextState)*/
                                     ? tmp->trueNode
                                     : falseNode);
                    } else {
                        MDDNode *nextNode = nullptr;
                        if(nexts.find(nextState) == nexts.end()) {
                            nextNode         = new MDDNode(nextState, nb++, node->level + 1, scope[i]->domain.maxSize());
                            nexts[nextState] = nextNode;
                        }
                        nextNode = nexts[nextState];
                        assert(nextNode != nullptr);
                        node->addChild(idv, nextNode);
                    }
                }
            }
        }
        prevs.swap(nexts);

        nexts.clear();
    }
    tmp->trueNode->id = nb++;
    tmp->nbNodes      = nb;

    return tmp;
}


//----------------------------------------------
// MDDNode
//----------------------------------------------


void MDDNode::addChild(int idv, MDDNode *target) {
    assert(idv >= 0 && idv < childs.size());
    childs[idv] = target;
    directAccessToChilds.push(idv);
}


void MDDNode::display() {
    int nb = 0;
    for(MDDNode *current : childs) {
        if(current != falseNode)
            std::cout << "(" << name << "," << nb << "," << current->name << ")";
        nb++;
    }
}


int MDDNode::nbChilds() const { return directAccessToChilds.size(); }


bool MDDNode::isRoot() const { return level == 0; }


MDDNode::MDDNode(std::string n, int _id, int lvl, int maxNbChilds) : id(_id), level(lvl), name(n) {
    childs.growTo(maxNbChilds, nullptr);
}
