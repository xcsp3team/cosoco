#ifndef FACTORYCONSTRAINTS_H
#define FACTORYCONSTRAINTS_H

#include "Variable.h"
#include "XCSP3Constants.h"
#include "constraints/extensions/MDDExtension.h"
#include "constraints/primitives/BasicNodes.h"
#include "utils/Verbose.h"
#include "xEqOryk.h"

namespace Cosoco {
using namespace XCSP3Core;

typedef struct Occurs Occurs;
struct Occurs {
    int       value;   // One of value (min,max) or Variables are uses
    int       min, max;
    Variable *x;
    int       type;
};

enum { OCCURS_INTEGER, OCCURS_INTERVAL, OCCURS_VARIABLE };

class BasicNode;

namespace FactoryConstraints {
//--------------------------------------------------------------------------------------
// Basic constraints
//--------------------------------------------------------------------------------------

Constraint *newExtensionConstraint(Problem *p, vec<Variable *> &vars, vec<vec<int>> &tuples, bool isSupport,
                                   bool hasStar = false);


void createConstraintExtensionAs(Problem *p, vec<Variable *> &vars, Constraint *c);

void createConstraintExtension(Problem *p, vec<Variable *> &vars, vec<vec<int>> &tuples, bool isSupport, bool hasStar = false);


void createConstraintXeqAndY(Problem *p, Variable *x, vec<Variable *> &l);

void createConstraintXeqXor(Problem *p, Variable *x, vec<Variable *> &l);

void createConstraintXeqGenOr(Problem *p, Variable *res, vec<Variable *> &vars, vec<BasicNode *> &nodes);

void createConstraintXeqGenAnd(Problem *p, Variable *res, vec<Variable *> &vars, vec<BasicNode *> &nodes);

void createConstraintGenOr(Problem *p, vec<Variable *> &vars, vec<BasicNode *> &nodes);


void createConstraintIntension(Problem *p, XCSP3Core::Tree *tree, vec<Variable *> &scope);

void createConstraintLessThan(Problem *p, Variable *x, int k, Variable *y, bool strict);


void createConstraintXeqYplusk(Problem *p, Variable *x, Variable *y, int k);


void createConstraintDistXYeqZ(Problem *p, Variable *x, Variable *y, Variable *z);

void createConstraintDistXYeqK(Problem *p, Variable *x, Variable *y, int k);

void createConstraintDistXYneK(Problem *p, Variable *x, Variable *y, int k);

void createConstraintXeqYeqK(Problem *p, Variable *x, Variable *y, int k);

void createConstraintXeqYneK(Problem *p, Variable *x, Variable *y, int k);

void createConstraintXeqKleY(Problem *p, Variable *x, Variable *y, int k);

void createConstraintXeqYleK(Problem *p, Variable *x, Variable *y, int k);

void createConstraintXeqMinSubY(Problem *p, Variable *x, Variable *y, int k);

void createConstraintMulEQ(Problem *p, Variable *x, Variable *y, Variable *z);

void createReification(Problem *p, Variable *x, Variable *y, Variable *z, ExpressionType op);

void createConstraintXor(Problem *p, vec<Variable *> &vars);

void createConstraintDoubleDiff(Problem *p, Variable *x1, Variable *x2, Variable *y1, Variable *y2);


void createConstraintMult(Problem *p, Variable *x, Variable *y, Variable *z);

void createConstraintUnary(Problem *p, Variable *x, vec<int> &values, bool isSupport);

void createConstraintUnaryGE(Problem *p, Variable *x, int k);


void createConstraintUnaryLE(Problem *p, Variable *x, int k);
//--------------------------------------------------------------------------------------
// Language constraints
//--------------------------------------------------------------------------------------

void createConstraintMDD(Problem *p, vec<Variable *> &vars, vec<XTransition *> &transitions);


void createConstraintMDD(Problem *p, vec<Variable *> &vars, MDD *mdd);


void createConstraintRegular(Problem *p, vec<Variable *> &vars, string start, std::vector<string> &final,
                             vec<XTransition *> &transitions);

//--------------------------------------------------------------------------------------
// Circuit constraints
//--------------------------------------------------------------------------------------

void createConstraintCircuit(Problem *p, vec<Variable *> &vars);

//--------------------------------------------------------------------------------------
// Comparison constraints
//--------------------------------------------------------------------------------------

void createConstraintAllDiff(Problem *p, vec<Variable *> &vars);

void createConstraintAllDiffExcept(Problem *p, vec<Variable *> &vars, vector<int> &except);

//----------------------------------------------------------------------


void createExtenstionDistinctVector(Problem *p, vec<Variable *> &list1, vec<Variable *> &list2);

void createConstraintAllDiffList(Problem *p, vec<vec<Variable *>> &lists);


//----------------------------------------------------------------------


void createConstraintAllEqual(Problem *p, vec<Variable *> &vars);

void createConstraintNotAllEqual(Problem *p, vec<Variable *> &vars);


//--------------------------------------------------------------------------------------
// Summing and counting constraints
//--------------------------------------------------------------------------------------

void createConstraintDiff(Problem *p, Variable *x, Variable *y, Variable *z);

void createConstraintSumBooleanEQ(Problem *p, vec<Variable *> &vars, long value);

void createConstraintSumBooleanLE(Problem *p, vec<Variable *> &vars, long value);

void createConstraintSumBooleanGE(Problem *p, vec<Variable *> &vars, long value);

void createConstraintSum(Problem *p, Variable *x, Variable *y, Variable *z);


void createConstraintSum(Problem *p, vec<Variable *> &vars, vec<Variable *> &coeffs, long l, OrderType order);

void createConstraintSum(Problem *p, vec<Variable *> &vars, vec<Variable *> &coeffs, Variable *z, OrderType order);

void createConstraintSum(Problem *p, vec<Variable *> &vars, vec<int> &coeffs, long l, OrderType order);

void createConstraintSumGen(Problem *p, vec<Variable *> &vars, vec<BasicNode *> &nodes, long l, OrderType order);

void createConstraintAtLeast(Problem *p, vec<Variable *> &vars, int value, int k);

void createConstraintAtMost(Problem *p, vec<Variable *> &vars, int value, int k);

void createConstraintAmong(Problem *p, vec<Variable *> &vars, vec<int> &value, int k);

void createConstraintExactly(Problem *p, vec<Variable *> &vars, int value, int k);

void createConstraintExactlyVariable(Problem *p, vec<Variable *> &vars, int value, Variable *k);

void createConstraintNValuesLE(Problem *p, vec<Variable *> &vars, int k);

void createConstraintNValuesGE(Problem *p, vec<Variable *> &vars, int k);

void createConstraintNValuesEQV(Problem *p, vec<Variable *> &vars, Variable *k);


//--------------------------------------------------------------------------------------
// Connection constraints
//--------------------------------------------------------------------------------------
void createConstraintElementConstant(Problem *p, vec<Variable *> &vars, Variable *index, int startIndex, int v);

void createConstraintElementVariable(Problem *p, vec<Variable *> &vars, Variable *index, int startIndex, Variable *v);

void createConstraintElementMatrix(Problem *p, vec<vec<Variable *>> &matrix, Variable *rindex, Variable *cindex, int value);

void createConstraintElementMatrix(Problem *p, vec<vec<Variable *>> &matrix, Variable *rindex, Variable *cindex, Variable *value);

void createConstraintCardinality(Cosoco::Problem *p, vec<Cosoco::Variable *> &vars, vec<int> &values, vec<Variable *> &occurs);

void createConstraintCardinality(Problem *p, vec<Variable *> &vars, vec<int> &values, vec<Occurs> &occurs);

void createConstraintOrdered(Problem *p, vec<Variable *> &vars, vector<int> &lengths, OrderType op);

void createConstraintOrdered(Problem *p, vec<Variable *> &vars, vec<Variable *> &lengths, OrderType op);

void createConstraintLex(Problem *p, vec<Variable *> &vars1, vec<Variable *> &vars2, OrderType op);


void createContraintChannelXY(Problem *p, vec<Variable *> &X, vec<Variable *> &Y, int startX, int startY);

void createConstraintChannel(Problem *p, vec<Variable *> &vars, int index);

//-----------------------------------------------------------------------

void createConstraintDisjunctive(Problem *p, Variable *x1, Variable *x2, int w1, int w2, Variable *aux);

void createConstraintDisjunctiveVars(Problem *p, Variable *x1, Variable *x2, Variable *w1, Variable *w2);

void createConstraintDisjunctive2D(Problem *p, Variable *x1, Variable *x2, Variable *y1, Variable *y2, int w1, int w2, int h1,
                                   int h2, Variable *aux);

void createConstraintDisjunctive2DVar(Problem *p, Variable *x1, Variable *x2, Variable *y1, Variable *y2, Variable *w1,
                                      Variable *w2, Variable *h1, Variable *h2,
                                      Variable *aux);   //-----------------------------------------------------------------------

void createConstraintMaximumLE(Problem *p, vec<Variable *> &vars, int k);


void createConstraintMaximumGE(Problem *p, vec<Variable *> &vars, int k);


void createConstraintMinimumLE(Problem *p, vec<Variable *> &vars, int k);

void createConstraintMinimumGE(Problem *p, vec<Variable *> &vars, int k);
void createConstraintMinimumEQ(Problem *p, vec<Variable *> &vars, int k);

void createConstraintMinimumVariableEQ(Problem *p, vec<Variable *> &vars, Variable *value);

void createConstraintMaximumVariableEQ(Problem *p, vec<Variable *> &vars, Variable *value);

void createConstraintMaximumArg(Problem *p, vec<Variable *> &vars, Variable *index, RankType rank);

//--------------------------------------------------------------------------------------
// Packing constraints
//--------------------------------------------------------------------------------------
void createConstraintCumulative(Problem *p, vec<Variable *> &vars, vec<int> &lengths, vec<int> &heights, Variable *limit);

void createConstraintCumulative(Problem *p, vec<Variable *> &vars, vec<int> &lengths, vec<int> &heights, int limit);

void createConstraintCumulativeHeightVariable(Problem *p, vec<Variable *> &vars, vec<int> &lengths, vec<Variable *> &heights,
                                              int limit);

void createConstraintCumulativeHeightVariableLV(Problem *p, vec<Variable *> &vars, vec<int> &lengths, vec<Variable *> &heights,
                                                Variable *limit);

void createConstraintCumulativeWidthVariables(Problem *p, vec<Variable *> &vars, vec<Variable *> &lengths, vec<int> &heights,
                                              int limit);

void createConstraintCumulativeHeightAndWidthVariables(Problem *p, vec<Variable *> &vars, vec<Variable *> &widths,
                                                       vec<Variable *> &heights, int limit);

void createConstraintCumulativeHeightAndWidthAndConditionVariables(Problem *p, vec<Variable *> &vars, vec<Variable *> &widths,
                                                                   vec<Variable *> &heights, Variable *limit);

void createConstraintNoOverlap(Problem *p, vec<Variable *> &X, vec<int> &width, vec<Variable *> &Y, vec<int> &heights);

void createConstraintPrecedence(Problem *p, vec<Variable *> &vars, vec<int> &values, bool covered);

void createConstraintBinPacking(Problem *p, vec<Variable *> &vars, vec<int> &sizes, vec<int> &limits);

void createConstraintBinPacking(Problem *p, vec<Variable *> &vars, vec<int> &sizes, vec<Variable *> &loads);

};   // namespace FactoryConstraints

}   // namespace Cosoco

#endif /* FACTORYCONSTRAINTS_H */
