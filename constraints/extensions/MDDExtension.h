#ifndef COSOCO_MDDEXTENSION_H
#define COSOCO_MDDEXTENSION_H


#include "Extension.h"
#include "solver/observers/ObserverDecision.h"
#include "structures/MDD.h"

namespace Cosoco {
class MDDExtension : public Extension, ObserverDeleteDecision {
   public:
    void notifyDeleteDecision(Variable *x, int v, Solver &s) override;
    MDDExtension(Problem &p, std::string n, vec<Variable *> &vars, vec<XCSP3Core::XTransition *> &transitions);
    MDDExtension(Problem &p, std::string n, vec<Variable *> &vars, MDD *m);

    bool filter(Variable *x) override;
    bool isSatisfiedBy(vec<int> &tuple) override;

    void attachSolver(Solver *s) override;

    bool isCorrectlyDefined() override;

    MDD *mdd;

   protected:
    vec<int> nbValuesWithoutSupports;
    int      nbTotalValuesWithoutSupports;
    int      trueTimestamp, falseTimestamp;
    vec<int> falseNodes, trueNodes;

    Matrix<bool> gac;   // a boolean for each literal (x,a) // TODO init??

    void beforeFiltering();
    bool exploreMDD(MDDNode *node);
    bool updateDomains();
    bool manageSuccessfulExploration(Variable *x, int idv, int level);
};
}   // namespace Cosoco

#endif   // COSOCO_MDDEXTENSION_H
