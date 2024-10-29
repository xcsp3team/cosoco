#ifndef FACTORYCONSTRAINTS_H
#define FACTORYCONSTRAINTS_H

#ifdef USE_XCSP3

#include "XCSP3Constants.h"
#include "XCSP3Tree.h"
#include "constraints/Constraint.h"
#include "constraints/extensions/BinaryExtension.h"
#include "constraints/extensions/CompactTable.h"
#include "constraints/extensions/MDDExtension.h"
#include "constraints/extensions/STRNeg.h"
#include "constraints/extensions/ShortSTR2.h"
#include "constraints/extensions/Unary.h"
#include "constraints/genericFiltering/AC3rm.h"
#include "constraints/globals/comparison/AllDifferent.h"
#include "constraints/globals/comparison/DistinctVectors.h"
#include "constraints/globals/comparison/Lexicographic.h"
#include "constraints/globals/comparison/NotAllEqual.h"
#include "constraints/globals/connection/Precedence.h"
#include "constraints/globals/connection/element/ElementConstant.h"
#include "constraints/globals/connection/element/ElementMatrix.h"
#include "constraints/globals/connection/element/ElementVariable.h"
#include "constraints/globals/connection/maximum/MaximumConstantGE.h"
#include "constraints/globals/connection/maximum/MaximumConstantLE.h"
#include "constraints/globals/connection/maximum/MaximumVariableEQ.h"
#include "constraints/globals/connection/minimum/MinimumConstantGE.h"
#include "constraints/globals/connection/minimum/MinimumConstantLE.h"
#include "constraints/globals/connection/minimum/MinimumVariableEQ.h"
#include "constraints/globals/counting/AtLeastK.h"
#include "constraints/globals/counting/AtMostK.h"
#include "constraints/globals/counting/CardinalityWeak.h"
#include "constraints/globals/counting/ExactlyK.h"
#include "constraints/globals/counting/ExactlyKVariable.h"
#include "constraints/globals/counting/NValuesEQVar.h"
#include "constraints/globals/counting/NValuesLEK.h"
#include "constraints/globals/graph/Circuit.h"
#include "constraints/globals/packing/BinPacking.h"
#include "constraints/globals/packing/BinPackingLoad.h"
#include "constraints/globals/packing/Cumulative.h"
#include "constraints/globals/packing/CumulativeVariablesC.h"
#include "constraints/globals/packing/CumulativeVariablesH.h"
#include "constraints/globals/packing/CumulativeVariablesHW.h"
#include "constraints/globals/packing/CumulativeVariablesHWC.h"
#include "constraints/globals/packing/CumulativeVariablesW.h"
#include "constraints/globals/packing/NoOverlap.h"
#include "constraints/globals/summing/Sum.h"
#include "constraints/globals/summing/SumEQ.h"
#include "constraints/globals/summing/SumGE.h"
#include "constraints/globals/summing/SumNE.h"
#include "constraints/globals/summing/SumScalarLEK.h"
#include "constraints/globals/summing/SumScalarLEVar.h"
#include "constraints/intension/Intension.h"
#include "constraints/primitives/DiffXY.h"
#include "constraints/primitives/Disjunctive.h"
#include "constraints/primitives/DisjunctiveVars.h"
#include "constraints/primitives/DistXYeqZ.h"
#include "constraints/primitives/EQ.h"
#include "constraints/primitives/GEUnary.h"
#include "constraints/primitives/LE.h"
#include "constraints/primitives/LEUnary.h"
#include "constraints/primitives/LT.h"
#include "constraints/primitives/Reification.h"
#include "constraints/primitives/xEqOryk.h"
#include "constraints/primitives/xTimesyEQz.h"
#include "mtl/Vec.h"
#include "solver/utils/Options.h"
#include "utils/Verbose.h"

namespace Cosoco {

typedef struct Occurs Occurs;
struct Occurs {
    int       value;   // One of value (min,max) or Variables are uses
    int       min, max;
    Variable *x;
    int       type;
};

enum { OCCURS_INTEGER, OCCURS_INTERVAL, OCCURS_VARIABLE };

static Verbose verbose;

class FactoryConstraints {
   public:
    //--------------------------------------------------------------------------------------
    // Basic constraints
    //--------------------------------------------------------------------------------------

    static void createConstraintXeqAndY(Problem *p, std::string name, Variable *x, vec<Variable *> &l) {
        l.push(x);
        p->addConstraint(new XeqAndY(*p, name, l));
    }

    static void createConstraintXeqOrYeqK(Problem *p, std::string name, Variable *res, vec<Variable *> &cl, vec<int> &vals) {
        p->addConstraint(new xEqOryk(*p, name, res, cl, vals));
    }

    static void createConstraintIntension(Problem *p, std::string name, XCSP3Core::Tree *tree, vec<Variable *> &scope) {
        p->addConstraint(new AdapterAC3rm(new Intension(*p, name, tree, scope)));
    }


    static void createConstraintLessThan(Problem *p, std::string name, Variable *x, int k, Variable *y, bool strict) {
        if(strict)
            p->addConstraint(new Lt(*p, name, x, y, k));
        else
            p->addConstraint(new Le(*p, name, x, y, k));
    }


    static void createConstraintXeqYplusk(Problem *p, std::string name, Variable *x, Variable *y, int k) {
        p->addConstraint(new EQ(*p, name, x, y, k));
    }


    static void createConstraintDistXYeqZ(Problem *p, std::string name, Variable *x, Variable *y, Variable *z) {
        p->addConstraint(new DistXYeqZ(*p, name, x, y, z));
    }

    static void createConstraintXeqYeqK(Problem *p, std::string name, Variable *x, Variable *y, int k) {
        p->addConstraint(new XeqYeqK(*p, name, x, y, k));
    }

    static void createConstraintXeqYneK(Problem *p, std::string name, Variable *x, Variable *y, int k) {
        p->addConstraint(new XeqYneK(*p, name, x, y, k));
    }

    static void createConstraintXeqKleY(Problem *p, std::string name, Variable *x, Variable *y, int k) {
        p->addConstraint(new XeqKleY(*p, name, x, y, k));
    }

    static void createConstraintXeqYleK(Problem *p, std::string name, Variable *x, Variable *y, int k) {
        p->addConstraint(new XeqYleK(*p, name, x, y, k));
    }

    static void createReification(Problem *p, std::string name, Variable *x, Variable *y, Variable *z,
                                  XCSP3Core::ExpressionType op) {
        assert(x != y && x != z && y != z);
        if(op == XCSP3Core::OLE) {
            p->addConstraint(new ReifLE(*p, name, x, y, z));
            return;
        }
        if(op == XCSP3Core::OLT) {
            p->addConstraint(new ReifLT(*p, name, x, y, z));
            return;
        }
        if(op == XCSP3Core::OEQ) {
            p->addConstraint(new ReifEQ(*p, name, x, y, z));
            return;
        }
        if(op == XCSP3Core::ONE) {
            p->addConstraint(new ReifNE(*p, name, x, y, z));
            return;
        }
        assert(false);
    }

    static void createConstraintMult(Problem *p, std::string name, Variable *x, Variable *y, Variable *z) {
        if(1 || x == y || x == z || y == z) {
            vec<Variable *> scp;
            if(x == y) {
                scp.push(x);
                scp.push(z);
                vec<vec<int>> tuples;
                for(int idv : x->domain) {
                    int v = x->domain.toVal(idv);
                    if(z->containsValue(v * v)) {
                        tuples.push();
                        tuples.last().push(idv);
                        tuples.last().push(z->domain.toIdv(v * v));
                    }
                }
                createConstraintExtension(p, name, scp, tuples, true);
                return;
            }
            if(x == z) {
                printf("TODO : y = 1 in mult");
                exit(1);
                return;
            }
            if(y == z) {
                printf("TODO : x = 1 in mult");
                exit(1);
                return;
            }
            scp.push(x);
            scp.push(y);
            scp.push(z);
            vec<vec<int>> tuples;
            for(int idv : x->domain) {
                int v = x->domain.toVal(idv);
                for(int idvy : y->domain) {
                    int v2 = y->domain.toVal(idvy);
                    if(z->containsValue(v * v2)) {
                        tuples.push();
                        tuples.last().push(idv);
                        tuples.last().push(idvy);
                        tuples.last().push(z->domain.toIdv(v * v2));
                    }
                }
            }
            createConstraintExtension(p, name, scp, tuples, true);


            return;
            printf("sdsd\n");
            scp.push(x);
            if(x != y)
                scp.push(y);
            if(x != z && y != z)
                scp.push(z);
            createConstraintIntension(p, name, new XCSP3Core::Tree("eq(" + z->_name + ",mul(" + x->_name + "," + y->_name + "))"),
                                      scp);
        } else
            p->addConstraint(new xTimesyEQz(*p, name, x, y, z));
    }

    static Constraint *newExtensionConstraint(Problem *p, std::string name, vec<Variable *> &vars, vec<vec<int>> &tuples,
                                              bool isSupport, bool hasStar = false) {
        Extension *ctr = nullptr;
        if(vars.size() == 2) {
            if(isSupport)
                ctr = new BinaryExtension(*p, name, isSupport, vars[0], vars[1]);
            else
                ctr = new STRNeg(*p, name, vars, tuples.size());
        } else {
            if(isSupport) {
                if(options::boolOptions["ct"].value)
                    ctr = new CompactTable(*p, name, vars, tuples.size());
                else
                    ctr = new ShortSTR2(*p, name, vars, tuples.size());
            } else {
                assert(hasStar == false);   // TODO
                ctr = new STRNeg(*p, name, vars, tuples.size());
            }
        }

        for(auto &tuple : tuples) ctr->addTuple(tuple);
        return ctr;
    }


    static void createConstraintExtension(Problem *p, std::string name, vec<Variable *> &vars, vec<vec<int>> &tuples,
                                          bool isSupport, bool hasStar = false) {
        p->addConstraint(newExtensionConstraint(p, name, vars, tuples, isSupport, hasStar));
    }


    static void createConstraintExtensionAs(Problem *p, std::string name, vec<Variable *> &vars, Constraint *c) {
        Extension *ctr            = nullptr;
        auto      *sameConstraint = (Extension *)c;
        assert(sameConstraint->scope.size() == vars.size());
        p->nbExtensionsSharded++;
        if(vars.size() == 1) {
            p->addConstraint(new Unary(*p, name, vars[0], ((Unary *)p->constraints.last())->values,
                                       ((Unary *)p->constraints.last())->areSupports));
            return;
        }

        if(vars.size() == 2) {
            if(sameConstraint->isSupport)
                ctr =
                    new BinaryExtension(*p, name, sameConstraint->isSupport, vars[0], vars[1], (BinaryExtension *)sameConstraint);
            else
                ctr = new STRNeg(*p, name, vars, sameConstraint->tuples);
        }
        if(vars.size() > 2) {
            if(sameConstraint->isSupport) {
                //    ctr = new ShortSTR2(*p, name, vars, sameConstraint->tuples);
                if(options::boolOptions["ct"].value)
                    ctr = new CompactTable(*p, name, vars, sameConstraint->tuples);
                else
                    ctr = new ShortSTR2(*p, name, vars, sameConstraint->tuples);
            } else
                ctr = new STRNeg(*p, name, vars, sameConstraint->tuples);
        }
        p->addConstraint(ctr);
    }


    static void createConstraintUnary(Problem *p, std::string name, Variable *x, vec<int> &values, bool isSupport) {
        p->addConstraint(new Unary(*p, name, x, values, isSupport));
    }


    static void createConstraintUnaryGE(Problem *p, std::string name, Variable *x, int k) {
        p->addConstraint(new GEUnary(*p, name, x, k));
    }


    static void createConstraintUnaryLE(Problem *p, std::string name, Variable *x, int k) {
        p->addConstraint(new LEUnary(*p, name, x, k));
    }
    //--------------------------------------------------------------------------------------
    // Language constraints
    //--------------------------------------------------------------------------------------

    static void createConstraintMDD(Problem *p, std::string name, vec<Variable *> &vars,
                                    vec<XCSP3Core::XTransition *> &transitions) {
        p->addConstraint(new MDDExtension(*p, name, vars, transitions));
    }


    static void createConstraintMDD(Problem *p, std::string name, vec<Variable *> &vars, MDD *mdd) {
        p->addConstraint(new MDDExtension(*p, name, vars, mdd));
    }


    static void createConstraintRegular(Problem *p, std::string name, vec<Variable *> &vars, string start,
                                        std::vector<string> &final, vec<XCSP3Core::XTransition *> &transitions) {
        p->addConstraint(new MDDExtension(*p, name, vars, MDD::buildFromAutomata(name, vars, start, final, transitions)));
    }

    //--------------------------------------------------------------------------------------
    // Circuit constraints
    //--------------------------------------------------------------------------------------

    static void createConstraintCircuit(Problem *p, std::string name, vec<Variable *> &vars) {
        p->addConstraint(new Circuit(*p, name, vars));
    }


    //--------------------------------------------------------------------------------------
    // Comparison constraints
    //--------------------------------------------------------------------------------------

    static void createConstraintAllDiff(Problem *p, std::string name, vec<Variable *> &vars) {
        if(vars.size() == 2)
            p->addConstraint(new DiffXY(*p, name, vars[0], vars[1]));
        else
            p->addConstraint(new AllDifferent(*p, name, vars));
    }


    static void createConstraintAllDiffExcept(Problem *p, std::string name, vec<Variable *> &vars, vector<int> &except) {
        for(int i = 0; i < vars.size(); i++) {
            Variable *x1 = vars[i];
            for(int j = 0; j < i; j++) {
                Variable     *x2 = vars[j];
                vec<vec<int>> tuples;
                for(int idv1 : x1->domain) {
                    int v1 = x1->domain.toVal(idv1);
                    if(std::find(except.begin(), except.end(), v1) != except.end())
                        continue;
                    for(int idv2 : x2->domain) {
                        int v2 = x2->domain.toVal(idv2);
                        if(std::find(except.begin(), except.end(), v2) != except.end())
                            continue;
                        if(v1 == v2) {
                            tuples.push();
                            vec<int> &t = tuples.last();
                            t.push(idv1);
                            t.push(idv2);
                        }
                    }
                }
                vec<Variable *> varsInConstraint;
                varsInConstraint.push(x1);
                varsInConstraint.push(x2);
                createConstraintExtension(p, name, varsInConstraint, tuples, false, false);
            }
        }
    }

    //----------------------------------------------------------------------


    static void createExtenstionDistinctVector(Problem *p, std::string name, vec<Variable *> &list1, vec<Variable *> &list2) {
        vec<Variable *> scope;
        vec<int>        position;
        position.growTo(p->nbVariables(), -1);

        for(Variable *x : list1) {
            if(position[x->idx] == -1) {
                scope.push(x);
                position[x->idx] = scope.size() - 1;
            }
        }
        for(Variable *x : list2) {
            if(position[x->idx] == -1) {
                scope.push(x);
                position[x->idx] = scope.size() - 1;
            }
        }

        vec<vec<int>> tuples;

        for(int i = 0; i < list1.size(); i++) {
            Variable *x = list1[i];
            Variable *y = list2[i];
            if(x == y)
                continue;
            for(int v1 : x->domain) {
                for(int v2 : y->domain) {
                    if(v1 == v2)
                        continue;
                    tuples.push();
                    vec<int> &tp = tuples.last();
                    tp.growTo(scope.size(), STAR);
                    tp[position[x->idx]] = x->domain.toIdv(v1);
                    tp[position[y->idx]] = y->domain.toIdv(v2);
                }
            }
        }
        createConstraintExtension(p, name, scope, tuples, true, true);
    }


    static void createConstraintAllDiffList(Problem *p, std::string name, vec<vec<Variable *>> &lists) {
        if(0) {
            verbose.log(NORMAL, "c AllDiff List constraint using %d DistinctVectors constraints \n",
                        lists.size() * (lists.size() - 1) / 2);

            for(int i = 0; i < lists.size(); i++)
                for(int j = i + 1; j < lists.size(); j++) p->addConstraint(new DistinctVectors(*p, name, lists[i], lists[j]));
        } else {
            verbose.log(NORMAL, "c AllDiff List constraint using extension constraint\n", lists.size() * (lists.size() - 1) / 2);
            for(int i = 0; i < lists.size(); i++)
                for(int j = i + 1; j < lists.size(); j++) createExtenstionDistinctVector(p, name, lists[i], lists[j]);
        }
    }


    //----------------------------------------------------------------------


    static void createConstraintAllEqual(Problem *p, std::string name, vec<Variable *> &vars) {
        for(int i = 0; i < vars.size() - 1; i++) p->addConstraint(new EQ(*p, name, vars[i], vars[i + 1]));
    }


    static void createConstraintNotAllEqual(Problem *p, std::string name, vec<Variable *> &vars) {
        p->addConstraint(new NotAllEqual(*p, name, vars));
    }


    //--------------------------------------------------------------------------------------
    // Summing and counting constraints
    //--------------------------------------------------------------------------------------

    static void createConstraintDiff(Problem *p, std::string name, Variable *x, Variable *y, Variable *z) {
        // w = y - z
        vec<Variable *> vars;
        vec<int>        coeffs;
        vars.push(y);
        vars.push(x);
        vars.push(z);
        coeffs.push(-1);
        coeffs.push(1);
        coeffs.push(1);
        p->addConstraint(new SumEQ(*p, name, vars, coeffs, 0));
    }

    static void createConstraintSum(Problem *p, std::string name, Variable *x, Variable *y, Variable *z) {   // x+y=z
        vec<Variable *> vars;
        vec<int>        coeffs;
        vars.push(z);
        vars.push(x);
        vars.push(y);
        coeffs.push(-1);
        coeffs.push(1);
        coeffs.push(1);
        p->addConstraint(new SumEQ(*p, name, vars, coeffs, 0));
    }


    static void createConstraintSum(Problem *p, std::string name, vec<Variable *> &vars, vec<Variable *> &coeffs, long l,
                                    XCSP3Core::OrderType order) {
        if(order == XCSP3Core::LE) {
            p->addConstraint(new SumScalarLEK(*p, name, vars, coeffs, l));
            return;
        }
        throw runtime_error("Not yet implemented scalar");
    }

    static void createConstraintSum(Problem *p, std::string name, vec<Variable *> &vars, vec<Variable *> &coeffs, Variable *z,
                                    XCSP3Core::OrderType order) {
        if(order == XCSP3Core::LE) {
            p->addConstraint(new SumScalarLEVar(*p, name, vars, coeffs, z));
            return;
        }
        throw runtime_error("Not yet implemented scalar");
    }

    static void createConstraintSum(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &coeffs, long l,
                                    XCSP3Core::OrderType order) {
        Sum *ctr = nullptr;

        // Rearrange coeffs.
        if(order == XCSP3Core::OrderType::LE || order == XCSP3Core::OrderType::LT) {
            for(int i = 0; i < coeffs.size(); i++) coeffs[i] = -coeffs[i];
            l = -l;
        }

        // Remove duplicate vars
        vec<Variable *> vars2;
        vec<int>        coefs2;
        for(int i = 0; i < vars.size(); i++) {
            int pos = vars2.containsRank(vars[i]);
            if(pos == -1) {
                vars2.push(vars[i]);
                coefs2.push(coeffs[i]);
            } else {
                coefs2[pos] += coeffs[i];
            }
        }
        vars2.copyTo(vars);
        coefs2.copyTo(coeffs);
        assert(coeffs.size() == vars.size());


        // Need order on coefficients
        for(int i = 0; i < vars.size(); i++) {
            int pos = i;
            for(int j = i + 1; j < vars.size(); j++) {
                if(coeffs[j] < coeffs[pos])
                    pos = j;
            }
            int tmp     = coeffs[i];
            coeffs[i]   = coeffs[pos];
            coeffs[pos] = tmp;
            Variable *x = vars[i];
            vars[i]     = vars[pos];
            vars[pos]   = x;
        }

        // remove coef 0
        int i = 0, j = 0;
        for(; i < coeffs.size(); i++)
            if(coeffs[i] != 0) {
                coeffs[j] = coeffs[i];
                vars[j++] = vars[i];
            }
        coeffs.shrink(i - j);
        vars.shrink(i - j);

        switch(order) {
            case XCSP3Core::OrderType::LE:
                ctr = new SumGE(*p, name, vars, coeffs, l);
                break;
            case XCSP3Core::OrderType::LT:
                ctr = new SumGE(*p, name, vars, coeffs, l + 1);
                break;
            case XCSP3Core::OrderType::GE:
                ctr = new SumGE(*p, name, vars, coeffs, l);
                break;
            case XCSP3Core::OrderType::GT:
                ctr = new SumGE(*p, name, vars, coeffs, l + 1);   // TODO
                break;
            case XCSP3Core::OrderType::IN:
                throw runtime_error("This is forbidden to construct a sum with IN operator");
                break;
            case XCSP3Core::OrderType::EQ:
                ctr = new SumEQ(*p, name, vars, coeffs, l);
                break;
            case XCSP3Core::OrderType::NE:
                ctr = new SumNE(*p, name, vars, coeffs, l);
                break;
        }
        assert(ctr != nullptr);
        p->addConstraint(ctr);
    }


    static void createConstraintAtLeast(Problem *p, std::string name, vec<Variable *> &vars, int value, int k) {
        if(k == 0)
            return;
        p->addConstraint(new AtLeastK(*p, name, vars, k, value));
    }


    static void createConstraintAtMost(Problem *p, std::string name, vec<Variable *> &vars, int value, int k) {
        p->addConstraint(new AtMostK(*p, name, vars, k, value));
    }


    static void createConstraintExactly(Problem *p, std::string name, vec<Variable *> &vars, int value, int k) {
        p->addConstraint(new ExactlyK(*p, name, vars, k, value));
    }


    static void createConstraintExactlyVariable(Problem *p, std::string name, vec<Variable *> &vars, int value, Variable *k) {
        p->addConstraint(new ExactlyKVariable(*p, name, vars, k, value));
    }


    static void createConstraintNValuesLE(Problem *p, std::string name, vec<Variable *> &vars, int k) {
        p->addConstraint(new NValuesLEK(*p, name, vars, k));
    }


    static void createConstraintNValuesEQV(Problem *p, std::string name, vec<Variable *> &vars, Variable *k) {
        p->addConstraint(new NValuesEQVar(*p, name, vars, k));
    }


    //--------------------------------------------------------------------------------------
    // Connection constraints
    //--------------------------------------------------------------------------------------
    static void createConstraintElementConstant(Problem *p, std::string name, vec<Variable *> &vars, Variable *index,
                                                int startIndex, int v) {
        p->addConstraint(new ElementConstant(*p, name, vars, index, v, startIndex == 1));
    }


    static void createConstraintElementVariable(Problem *p, std::string name, vec<Variable *> &vars, Variable *index,
                                                int startIndex, Variable *v) {
        p->addConstraint(new ElementVariable(*p, name, vars, index, v, startIndex == 1));
    }


    static void createConstraintElementMatrix(Problem *p, std::string name, vec<vec<Variable *>> &matrix, Variable *rindex,
                                              Variable *cindex, int value) {
        p->addConstraint(new ElementMatrix(*p, name, matrix, rindex, cindex, value));
    }


    static void createConstraintCardinality(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &values,
                                            vec<Occurs> &occurs) {
        verbose.log(DEBUGVERBOSE,
                    "c Create Cardinality(vars,values,occurs) constraint using %d Exactly/AtMost.. constraint \nc\n",
                    vars.size());
        if(occurs.size() != values.size())
            throw std::logic_error("Cardinality: Occurs and values must have the same size");

        if(occurs[0].type == OCCURS_INTEGER && vars.size() > 1000) {
            vec<int> occs;
            for(Occurs &o : occurs) occs.push(o.value);
            p->addConstraint(new CardinalityWeak(*p, name, vars, values, occs));
            return;
        }

        for(int i = 0; i < occurs.size(); i++) {
            if(occurs[i].type == OCCURS_INTEGER)
                createConstraintExactly(p, name + "card as exactly", vars, values[i], occurs[i].value);
            if(occurs[i].type == OCCURS_INTERVAL) {
                createConstraintAtLeast(p, name + "card as atleast", vars, values[i], occurs[i].min);
                createConstraintAtMost(p, name + "card as atleast", vars, values[i], occurs[i].max);
            }
            if(occurs[i].type == OCCURS_VARIABLE) {
                createConstraintExactlyVariable(p, name + "card as exactly", vars, values[i], occurs[i].x);
            }
        }
    }


    static void createConstraintOrdered(Problem *p, std::string name, vec<Variable *> &vars, vector<int> &lengths,
                                        XCSP3Core::OrderType op) {
        for(int i = 0; i < vars.size() - 1; i++) {
            int k = lengths.size() == 0 ? 0 : lengths[i];
            if(op == XCSP3Core::OrderType::LE)
                p->addConstraint(
                    new Le(*p, name + vars[i]->name() + " op " + name + vars[i + 1]->name(), vars[i], vars[i + 1], k));
            if(op == XCSP3Core::OrderType::LT)
                p->addConstraint(
                    new Lt(*p, name + vars[i]->name() + " op " + name + vars[i + 1]->name(), vars[i], vars[i + 1], k));

            if(op == XCSP3Core::OrderType::GE)
                p->addConstraint(
                    new Le(*p, name + vars[i]->name() + " op " + name + vars[i + 1]->name(), vars[i + 1], vars[i], -k));
            if(op == XCSP3Core::OrderType::GT)
                p->addConstraint(
                    new Lt(*p, name + vars[i]->name() + " op " + name + vars[i + 1]->name(), vars[i + 1], vars[i], -k));
        }
    }


    static void createConstraintLex(Problem *p, std::string name, vec<Variable *> &vars1, vec<Variable *> &vars2,
                                    XCSP3Core::OrderType op) {
        if(op == XCSP3Core::OrderType::LE)
            p->addConstraint(new Lexicographic(*p, name, vars1, vars2, false));
        if(op == XCSP3Core::OrderType::LT)
            p->addConstraint(new Lexicographic(*p, name, vars1, vars2, true));

        if(op == XCSP3Core::OrderType::GE)
            p->addConstraint(new Lexicographic(*p, name, vars2, vars1, false));
        if(op == XCSP3Core::OrderType::GT)
            p->addConstraint(new Lexicographic(*p, name, vars2, vars1, true));
    }


    static void createContraintChannelXY(Problem *p, std::string name, vec<Variable *> &X, vec<Variable *> &Y, int startX,
                                         int startY) {
        verbose.log(NORMAL, "c Create channel(X,Y) constraint using %d element constraint \n", X.size());
        for(int i = 0; i < X.size(); i++) createConstraintElementConstant(p, name, Y, X[i], startY, i);
    }

    static void createConstraintChannel(Problem *p, string name, vec<Variable *> &vars, int index) {
        assert(index == 0);
        createConstraintAllDiff(p, name, vars);
        for(int i = 0; i < vars.size(); i++) {
            createConstraintElementConstant(p, name, vars, vars[i], 0, i);
        }
    }

    //-----------------------------------------------------------------------

    static void createConstraintNoOverlap(Problem *p, std::string name, Variable *x1, Variable *x2, int w1, int w2) {
        p->addConstraint(new Disjunctive(*p, name, x1, x2, w1, w2));
        // string tmp = "or(le(add(" + x1->_name + "," + to_string(w1) + ")," + x2->_name + "),le(add(" + x2->_name + "," +
        // to_string(w2) + ")," + x1->_name + "))"; createConstraintIntension(p, "no overlap", tmp);
    }

    static void createConstraintDisjunctiveVars(Problem *p, std::string name, Variable *x1, Variable *x2, Variable *w1,
                                                Variable *w2) {
        p->addConstraint(new DisjunctiveVars(*p, name, x1, x2, w1, w2));
    }

    static void createConstraintDisjunctive2D(Problem *p, std::string name, Variable *x1, Variable *x2, Variable *y1,
                                              Variable *y2, int w1, int w2, int h1, int h2) {
        p->addConstraint(new Disjunctive2D(*p, name, x1, x2, y1, y2, w1, w2, h1, h2));
    }
    //-----------------------------------------------------------------------

    static void createConstraintMaximumLE(Problem *p, std::string name, vec<Variable *> &vars, int k) {
        p->addConstraint(new MaximumConstantLE(*p, name, vars, k));
    }


    static void createConstraintMaximumGE(Problem *p, std::string name, vec<Variable *> &vars, int k) {
        p->addConstraint(new MaximumConstantGE(*p, name, vars, k));
    }


    static void createConstraintMinimumLE(Problem *p, std::string name, vec<Variable *> &vars, int k) {
        p->addConstraint(new MinimumConstantLE(*p, name, vars, k));
    }


    static void createConstraintMinimumGE(Problem *p, std::string name, vec<Variable *> &vars, int k) {
        p->addConstraint(new MinimumConstantGE(*p, name, vars, k));
    }


    static void createConstraintMinimumVariableEQ(Problem *p, std::string name, vec<Variable *> &vars, Variable *value) {
        if(vars.contains(value)) {
            verbose.log(NORMAL, "c min(x...)=x -> use inequalities\n");
            for(Variable *x : vars) {
                if(x != value)
                    p->addConstraint(new Le(*p, name, value, x, 0));
            }
        } else
            p->addConstraint(new MinimumVariableEQ(*p, name, vars, value));
    }


    static void createConstraintMaximumVariableEQ(Problem *p, std::string name, vec<Variable *> &vars, Variable *value) {
        if(vars.contains(value)) {
            verbose.log(NORMAL, "c max(x...)=x -> use inequalities\n");
            for(Variable *x : vars) {
                if(x != value)
                    p->addConstraint(new Le(*p, name, x, value, 0));
            }
        } else
            p->addConstraint(new MaximumVariableEQ(*p, name, vars, value));
    }


    //--------------------------------------------------------------------------------------
    // Packing constraints
    //--------------------------------------------------------------------------------------
    static void createConstraintCumulative(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &lengths,
                                           vec<int> &heights, Variable *limit) {
        p->addConstraint(new CumulativeVariablesC(*p, name, vars, lengths, heights, limit));
    }

    static void createConstraintCumulative(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &lengths,
                                           vec<int> &heights, int limit) {
        p->addConstraint(new Cumulative(*p, name, vars, vars, lengths, heights, limit));
    }


    static void createConstraintCumulativeHeightVariable(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &lengths,
                                                         vec<Variable *> &heights, int limit) {
        p->addConstraint(new CumulativeVariablesH(*p, name, vars, lengths, heights, limit));
    }

    static void createConstraintCumulativeHeightVariableLV(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &lengths,
                                                           vec<Variable *> &heights, Variable *limit) {
        p->addConstraint(new CumulativeVariablesHLimitV(*p, name, vars, lengths, heights, limit));
    }

    static void createConstraintCumulativeWidthVariables(Problem *p, std::string name, vec<Variable *> &vars,
                                                         vec<Variable *> &lengths, vec<int> &heights, int limit) {
        p->addConstraint(new CumulativeVariablesW(*p, name, vars, lengths, heights, limit));
    }

    static void createConstraintCumulativeHeightAndWidthVariables(Problem *p, std::string name, vec<Variable *> &vars,
                                                                  vec<Variable *> &widths, vec<Variable *> &heights, int limit) {
        p->addConstraint(new CumulativeVariablesHW(*p, name, vars, widths, heights, limit));
    }

    static void createConstraintCumulativeHeightAndWidthAndConditionVariables(Problem *p, std::string name, vec<Variable *> &vars,
                                                                              vec<Variable *> &widths, vec<Variable *> &heights,
                                                                              Variable *limit) {
        p->addConstraint(new CumulativeVariablesHWC(*p, name, vars, widths, heights, limit));
    }

    static void createConstraintNoOverlap(Problem *p, std::string name, vec<Variable *> &X, vec<int> &width, vec<Variable *> &Y,
                                          vec<int> &heights) {
        p->addConstraint(new NoOverlap(*p, name, X, width, Y, heights));
    }


    static void createConstraintPrecedence(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &values, bool covered) {
        p->addConstraint(new Precedence(*p, name, vars, values, covered));
    }

    static void createConstraintBinPacking(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &sizes,
                                           vec<int> &limits) {
        p->addConstraint(new BinPacking(*p, name, vars, sizes, limits));
    }

    static void createConstraintBinPacking(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &sizes,
                                           vec<Variable *> &loads) {
        p->addConstraint(new BinPackingLoad(*p, name, vars, sizes, loads));
    }
};

}   // namespace Cosoco

#endif /* USE_XCSP3 */

#endif /* FACTORYCONSTRAINTS_H */
