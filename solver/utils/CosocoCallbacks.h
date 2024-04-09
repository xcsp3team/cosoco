#ifndef COSOCO_COSOCOCALLBACKS_H
#define COSOCO_COSOCOCALLBACKS_H

#include <numeric>
#include <utility>

#include "FactoryConstraints.h"
#include "XCSP3Constants.h"
#include "XCSP3Constraint.h"
#include "XCSP3CoreCallbacks.h"
#include "constraints/extensions/structures/MDD.h"
#include "core/OptimizationProblem.h"
#include "core/Problem.h"
#include "core/domain/DomainRange.h"
#include "core/domain/DomainValues.h"
#include "utils/Utils.h"


using namespace Cosoco;
using namespace std;
namespace XCSP3Core {
class CosocoCallbacks;


class Primitive {
   public:
    Tree                       *canonized, *pattern;
    std::vector<int>            constants;
    std::vector<std::string>    variables;
    std::vector<ExpressionType> operators;
    XCSP3Core::CosocoCallbacks &callbacks;
    int                         arity;

    std::string id;


    Primitive(XCSP3Core::CosocoCallbacks &m, string expr, int a) : pattern(new Tree(std::move(expr))), callbacks(m), arity(a) { }
    Primitive(XCSP3Core::CosocoCallbacks &m) : pattern(nullptr), callbacks(m), arity(0) { }
    virtual ~Primitive() { }


    Primitive *setTarget(std::string _id, Tree *c) {
        id        = _id;
        canonized = c;
        return this;
    }


    virtual bool post() = 0;


    virtual bool match() {
        constants.clear();
        variables.clear();
        operators.clear();
        return arity == canonized->arity() && Node::areSimilar(canonized->root, pattern->root, operators, constants, variables) &&
               post();
    }
};


class ManageIntension {
   public:
    vec<Primitive *>          patterns;
    CosocoCallbacks          &callbacks;
    map<string, Constraint *> cachedExtensions;
    explicit ManageIntension(CosocoCallbacks &callbacks);
    void createPrimitives();
    void intension(std::string id, Tree *tree);
    bool recognizePrimitives(std::string id, Tree *tree);
    bool toExtension(std::string id, Tree *tree, vec<Variable *> &scope);
    bool decompose(std::string id, Tree *tree);
    bool decompose(Node *node);
    void extractVariables(Node *node, vector<string> &listOfVariables);
};


class CosocoCallbacks : public XCSP3CoreCallbacks {
    friend class ManageIntension;

   protected:
    vec<int> &vector2vec(vector<int> &src) {
        vals.clear();
        vals.growTo(src.size());
        for(unsigned int i = 0; i < src.size(); i++) vals[i] = src[i];
        return vals;
    }


    vec<Variable *> &toMyVariables(vector<XVariable *> &src, vec<Variable *> &dest) {
        dest.clear();
        for(unsigned int i = 0; i < src.size(); i++) dest.push(problem->mapping[src[i]->id]);
        return dest;
    }


    vec<Variable *> &toMyVariables(vector<XVariable *> &src) { return toMyVariables(src, vars); }


    Range possibleValuesForExpressionInRange(Node *node) {
        if(XCSP3Core::isPredicateOperator(node->type))
            return {0, 1};

        if(node->type == OVAR) {
            auto     *nx = dynamic_cast<NodeVariable *>(node);
            Variable *x  = problem->mapping[nx->var];
            return {x->minimum(), x->maximum()};
        }
        if(node->type == ODECIMAL) {
            auto *nx = dynamic_cast<NodeConstant *>(node);
            return {nx->val, nx->val};
        }

        assert(node->parameters.size() > 0);   // To be sure
        assert(node->parameters.size() > 0);   // To be sure
        vec<Range> ranges;

        for(Node *n : node->parameters) ranges.push(possibleValuesForExpressionInRange(n));

        if(node->type == ONEG)
            return ranges[0].negRange();

        if(node->type == OABS)
            return ranges[0].absRange();

        if(node->type == OSQR)
            return {ranges[0].min * ranges[0].min, ranges[0].max * ranges[0].max};

        assert(ranges.size() > 1);

        if(node->type == OSUB)
            return ranges[0].addRange(ranges[1].negRange());

        if(node->type == ODIV) {
            return ranges[0];
        }

        if(node->type == OMOD) {
            return {0, ranges[1].max};
        }


        if(node->type == OPOW) {
            assert(false);
        }

        if(node->type == ODIST)
            return ranges[0].addRange(ranges[1].negRange()).absRange();


        if(node->type == OADD) {
            int mn = 0, mx = 0;
            for(Range r : ranges) {
                mn += r.min;
                mx += r.max;
            }
            return {mn, mx};
        }


        if(node->type == OMUL) {
            int a, b, c, d;
            int min = ranges[0].min;
            int max = ranges[0].max;
            for(int i = 1; i < ranges.size(); i++) {
                a   = min * ranges[i].min;
                b   = min * ranges[i].max;
                c   = max * ranges[i].min;
                d   = max * ranges[i].max;
                min = std::min({a, b, c, d});
                max = std::max({a, b, c, d});
            }
            return {min, max};
        }


        if(node->type == OMIN) {
            int mn = ranges[0].min, mx = ranges[0].max;
            for(Range &r : ranges) {
                mn = mn > r.min ? r.min : mn;
                mx = mx > r.max ? r.max : mx;
            }
            return {mn, mx};
        }

        if(node->type == OMAX) {
            int mn = ranges[0].min, mx = ranges[0].max;
            for(Range &r : ranges) {
                mn = mn < r.min ? r.min : mn;
                mx = mx < r.max ? r.max : mx;
            }
            return {mn, mx};
        }


        if(node->type == OIF) {
            assert(false);
        }

        assert(false);
        return Range(0, 0);   // Avoid warning
    }


    void possibleValuesForExpression(Tree *tree, std::map<std::string, int> &tuple, set<int> &values, int idx) {
        if(((unsigned int)idx) == tree->listOfVariables.size()) {
            values.insert(tree->evaluate(tuple));
            return;
        }
        Variable *x = problem->mapping[tree->listOfVariables[idx]];   // Take core 0 is ok
        for(int idv : x->domain) {
            tuple[tree->listOfVariables[idx]] = x->domain.toVal(idv);
            possibleValuesForExpression(tree, tuple, values, idx + 1);
        }
    }


    bool                insideGroup;
    int                 nbIntension;
    bool                inArray;
    int                 i2eNumber;
    int                 nbMDD;
    vec<Variable *>     vars;   // Not so beautiful, but efficient...
    vec<int>            vals;   // Avoid std::move and save lines...
    bool                startToParseObjective;
    int                 auxiliaryIdx;
    map<string, string> expressionsToAuxiliaryVariables;
    vector<XVariable *> previousArgument;
    int                 nbIntension2Extention, nbSharedIntension2Extension;

   public:
    int                    nbcores;
    unsigned long long     intension2extensionLimit;
    vec<Cosoco::Problem *> problems;
    Cosoco::Problem       *problem;
    bool                   optimizationProblem;
    bool invertOptimization;   // See Sum objective. If minimize -> Maximize and change sum (only sumGE is supported)
    vec<vec<Variable *>>               decisionVariables;
    int                                nbInitialsVariables;
    std::map<std::string, XVariable *> mappingXV;
    ManageIntension                   *manageIntension;

    CosocoCallbacks(int ncores, unsigned long long i2e)
        : startToParseObjective(false), nbcores(ncores), intension2extensionLimit(i2e) {
        recognizeSpecialIntensionCases = false;
        manageIntension                = new ManageIntension(*this);
    }

    void beginInstance(InstanceType type) override;

    void endInstance() override;

    void buildVariableInteger(string id, int minValue, int maxValue) override;

    void buildVariableInteger(string id, vector<int> &values) override;

    void beginVariableArray(string id) override;

    void endVariableArray() override;

    void endVariables() override;

    void initGroups();

    void beginGroup(string name) override;

    void endGroup() override;

    void beginSlide(string id, bool circular) override;

    void endSlide() override;

    void beginBlock(string classes) override;

    void endBlock() override;

    void beginObjectives() override;


    //--------------------------------------------------------------------------------------
    // Basic constraints
    //--------------------------------------------------------------------------------------


    void buildConstraintExtension(string id, vector<XVariable *> list, vector<vector<int>> &origTuples, bool support,
                                  bool hasStar) override;


    void buildConstraintExtensionAs(string id, vector<XVariable *> list, bool support, bool hasStar);

    void buildConstraintExtension2(const string &id, vec<Variable *> &scope, const vector<vector<int>> &origTuples, bool support,
                                   bool hasStar) const;

    void buildConstraintExtension(string id, XVariable *variable, vector<int> &tuples, bool support, bool hasStar) override;

    void buildConstraintIntension(string id, Tree *tree) override;

    void buildConstraintPrimitive(string id, OrderType op, XVariable *x, int k, XVariable *y) override;

    void buildConstraintPrimitive(string id, OrderType op, XVariable *xx, int k) override;   // x op k op is <= or >=

    void buildConstraintPrimitive(string id, XVariable *xx, bool in, int min,
                                  int max) override;   // x in/notin [min,max]

    void buildConstraintMult(string id, XVariable *x, XVariable *y, XVariable *z) override;


    //--------------------------------------------------------------------------------------
    // Language  constraints
    //--------------------------------------------------------------------------------------

    Cosoco::MDD *sameMDDAsPrevious(vec<Variable *> &list);

    void buildConstraintMDD(string id, vector<XVariable *> &list, vector<XTransition> &transitions) override;

    void buildConstraintRegular(string id, vector<XVariable *> &list, string start, vector<string> &final,
                                vector<XTransition> &transitions) override;

    //--------------------------------------------------------------------------------------
    // Graph constraints
    //--------------------------------------------------------------------------------------

    void buildConstraintCircuit(string id, vector<XVariable *> &list, int startIndex) override;

    //--------------------------------------------------------------------------------------
    // Comparison constraints
    //--------------------------------------------------------------------------------------

    void buildConstraintAlldifferent(string id, vector<XVariable *> &list) override;

    void buildConstraintAlldifferentExcept(string id, vector<XVariable *> &list, vector<int> &except) override;

    void buildConstraintAlldifferent(string id, vector<Tree *> &trees) override;

    void buildConstraintAlldifferentList(string id, vector<vector<XVariable *>> &origlists) override;

    void buildConstraintAlldifferentMatrix(string id, vector<vector<XVariable *>> &matrix) override;

    void buildConstraintAllEqual(string id, vector<XVariable *> &list) override;

    void buildConstraintNotAllEqual(string id, vector<XVariable *> &list) override;

    void buildConstraintOrdered(string id, vector<XVariable *> &list, OrderType order) override;

    void buildConstraintOrdered(string id, vector<XVariable *> &list, vector<int> &lengths, OrderType order) override;

    void buildConstraintLex(string id, vector<vector<XVariable *>> &lists, OrderType order) override;

    void buildConstraintLexMatrix(string id, vector<vector<XVariable *>> &matrix, OrderType order) override;


    //--------------------------------------------------------------------------------------
    // Summing and counting constraints
    //--------------------------------------------------------------------------------------

    void buildConstraintSum(string id, vector<XVariable *> &list, XCondition &xc) override;

    void buildConstraintSum(string id, vector<XVariable *> &list, vector<int> &origcoeffs, XCondition &xc) override;

    void buildConstraintSum(string id, vec<Variable *> &variables, vector<int> &coeffs, XCondition &xc);

    void buildConstraintSum(string id, vector<XVariable *> &list, vector<XVariable *> &coeffs, XCondition &xc) override;

    void buildConstraintSum(string id, vector<Tree *> &trees, XCondition &cond) override;

    void buildConstraintSum(string id, vector<Tree *> &trees, vector<int> &coefs, XCondition &cond) override;

    void buildConstraintAtMost(string id, vector<XVariable *> &list, int value, int k) override;

    void buildConstraintAtLeast(string id, vector<XVariable *> &list, int value, int k) override;

    void buildConstraintExactlyK(string id, vector<XVariable *> &list, int value, int k) override;

    void buildConstraintExactlyVariable(string id, vector<XVariable *> &list, int value, XVariable *x) override;

    void buildConstraintNValues(string id, vector<XVariable *> &list, XCondition &xc) override;

    void buildConstraintCount(string id, vector<XVariable *> &list, vector<XVariable *> &values, XCondition &xc) override;

    void buildConstraintCount(string id, vector<Tree *> &trees, vector<int> &values, XCondition &xc) override;

    void buildConstraintCount(string id, vector<Tree *> &trees, vector<XVariable *> &values, XCondition &xc) override;

    void buildConstraintCardinality(string id, vector<XVariable *> &list, vector<int> values, vector<int> &intOccurs,
                                    bool closed) override;

    void buildConstraintCardinality(string id, vector<XVariable *> &list, vector<int> values, vector<XVariable *> &varOccurs,
                                    bool closed) override;

    void buildConstraintCardinality(string id, vector<XVariable *> &list, vector<int> values, vector<XInterval> &intervalOccurs,
                                    bool closed) override;

    //--------------------------------------------------------------------------------------
    // Connection constraints
    //--------------------------------------------------------------------------------------

    static string createExpression(string minmax, OrderType op, vector<XVariable *> &list, string value);

    void buildConstraintMaximum(string id, vector<Tree *> &list, XCondition &xc) override;

    void buildConstraintMinimum(string id, vector<Tree *> &list, XCondition &xc) override;

    void buildConstraintMaximum(string id, vector<XVariable *> &list, XCondition &xc) override;

    void buildConstraintMinimum(string id, vector<XVariable *> &list, XCondition &xc) override;

    void buildConstraintElement(string id, vector<XVariable *> &list, int startIndex, XVariable *index, RankType rank,
                                int value) override;

    void buildConstraintElement(string id, vector<XVariable *> &list, int startIndex, XVariable *index, RankType rank,
                                XVariable *value) override;

    void buildConstraintElement(string id, vector<int> &list, int startIndex, XVariable *index, RankType rank,
                                XVariable *value) override;

    void buildConstraintElement(string id, vector<XVariable *> &list, XVariable *index, int startIndex, XCondition &xc) override;

    void buildConstraintElement(string id, vector<vector<int>> &matrix, int startRowIndex, XVariable *rowIndex, int startColIndex,
                                XVariable *colIndex, XVariable *value) override;


    void buildConstraintElement(string id, vector<vector<XVariable *>> &matrix, int startRowIndex, XVariable *rowIndex,
                                int startColIndex, XVariable *colIndex, int value) override;


    void buildConstraintChannel(string id, vector<XVariable *> &list1, int startIndex1, vector<XVariable *> &list2,
                                int startIndex2) override;

    void buildConstraintChannel(string id, vector<XVariable *> &list, int startIndex, XVariable *value) override;

    //--------------------------------------------------------------------------------------
    // packing and schedulling constraints
    //--------------------------------------------------------------------------------------


    void buildConstraintNoOverlap(string id, vector<XVariable *> &origins, vector<int> &lengths, bool zeroIgnored) override;

    void buildConstraintNoOverlap(string id, vector<vector<XVariable *>> &origins, vector<vector<XVariable *>> &lengths,
                                  bool zeroIgnored) override;

    void buildConstraintNoOverlap(string id, vector<vector<XVariable *>> &origins, vector<vector<int>> &lengths,
                                  bool zeroIgnored) override;

    void buildConstraintCumulative(string id, vector<XVariable *> &origins, vector<int> &lengths, vector<int> &heights,
                                   XCondition &xc) override;

    void buildConstraintCumulative(string id, vector<XVariable *> &origins, vector<int> &lengths, vector<XVariable *> &varHeights,
                                   XCondition &xc) override;

    void buildConstraintCumulative(string id, vector<XVariable *> &origins, vector<XVariable *> &varlengths, vector<int> &heights,
                                   XCondition &xc) override;
    void buildConstraintCumulative(string id, vector<XVariable *> &origins, vector<XVariable *> &lengths,
                                   vector<XVariable *> &heights, XCondition &xc) override;
    void buildConstraintBinPacking(string id, vector<XVariable *> &list, vector<int> &sizes, XCondition &cond) override;

    void buildConstraintBinPacking(string id, vector<XVariable *> &list, vector<int> &sizes, vector<int> &capacities,
                                   bool load) override;

    void buildConstraintBinPacking(string id, vector<XVariable *> &list, vector<int> &sizes, vector<XVariable *> &capacities,
                                   bool load) override;

    void buildConstraintBinPacking(string id, vector<XVariable *> &list, vector<int> &sizes, vector<XCondition> &conditions,
                                   int startindex) override;

    //--------------------------------------------------------------------------------------
    // Instantiation constraint
    //--------------------------------------------------------------------------------------

    void buildConstraintInstantiation(string id, vector<XVariable *> &list, vector<int> &values) override;

    void buildConstraintPrecedence(string id, vector<XVariable *> &list, vector<int> values, bool covered) override;

    void buildConstraintPrecedence(string id, vector<XVariable *> &list, bool covered) override;

    void buildConstraintKnapsack(string id, vector<XVariable *> &list, vector<int> &weights, vector<int> &profits,
                                 XCondition weightsCondition, XCondition &profitCondition) override;

    void buildConstraintFlow(string id, vector<XVariable *> &list, vector<int> &balance, vector<int> &weights,
                             vector<vector<int>> &arcs, XCondition &xc) override;

    //--------------------------------------------------------------------------------------
    // Objectives
    //--------------------------------------------------------------------------------------

    void buildObjectiveMinimizeExpression(string expr) override;

    void buildObjectiveMaximizeExpression(string expr) override;

    void buildObjectiveMinimizeVariable(XVariable *x) override;

    void buildObjectiveMaximizeVariable(XVariable *x) override;

    void buildObjectiveMinimize(ExpressionObjective type, vector<XVariable *> &list) override;

    void buildObjectiveMaximize(ExpressionObjective type, vector<XVariable *> &list) override;

    void buildObjectiveMinimize(ExpressionObjective type, vec<Variable *> &variables, vector<int> &origcoeffs);

    void buildObjectiveMinimize(ExpressionObjective type, vector<XVariable *> &list, vector<int> &origcoeffs) override;

    void buildObjectiveMaximize(ExpressionObjective type, vec<Variable *> &variables, vector<int> &origcoeffs);

    void buildObjectiveMaximize(ExpressionObjective type, vector<XVariable *> &list, vector<int> &origcoeffs) override;

    void createAuxiliaryVariablesAndExpressions(vector<Tree *> &trees, vector<string> &auxiliaryVariables);

    void buildObjectiveMinimize(ExpressionObjective type, vector<Tree *> &trees, vector<int> &coefs) override;

    void buildObjectiveMaximize(ExpressionObjective type, vector<Tree *> &trees, vector<int> &coefs) override;

    void buildObjectiveMinimize(ExpressionObjective type, vector<Tree *> &trees) override;

    void buildObjectiveMaximize(ExpressionObjective type, vector<Tree *> &trees) override;

    void buildAnnotationDecision(vector<XVariable *> &list) override;

    void buildConstraintChannel(string id, vector<XVariable *> &list, int startIndex) override;

    void buildObjectiveMinimize(ExpressionObjective type, vector<XVariable *> &list, vector<XVariable *> &coefs) override;

    void buildObjectiveMaximize(ExpressionObjective type, vector<XVariable *> &list, vector<XVariable *> &coefs) override;

    void buildConstraintNoOverlap(string id, vector<XVariable *> &origins, vector<XVariable *> &lengths,
                                  bool zeroIgnored) override;
};


}   // namespace XCSP3Core
#endif   // COSOCO_COSOCOCALLBACKS_H
