//
// Created by audemard on 14/03/25.
//
#include "FactoryConstraints.h"

#include <BinaryExtensionSupport.h>
#include <NoGood.h>
#include <STR0.h>

#include <iostream>
#include <regex>

#include "Among.h"
#include "BinPacking.h"
#include "BinPackingLoad.h"
#include "CardinalityWeak.h"
#include "CompactTable.h"
#include "Constraint.h"
#include "CumulativeVariablesC.h"
#include "CumulativeVariablesH.h"
#include "CumulativeVariablesHW.h"
#include "CumulativeVariablesHWC.h"
#include "CumulativeVariablesW.h"
#include "DisjunctiveVars.h"
#include "Options.h"
#include "Precedence.h"
#include "Reification.h"
#include "XCSP3Constants.h"
#include "constraints/globals/connection/element/ElementMatrix.h"
#include "constraints/globals/connection/maximum/MaximumVariableEQ.h"
#include "constraints/globals/counting/NValuesEQVar.h"
#include "constraints/globals/graph/Circuit.h"
#include "constraints/globals/packing/Cumulative.h"
#include "constraints/globals/packing/NoOverlap.h"
#include "constraints/globals/summing/SumScalarLEK.h"
#include "constraints/globals/summing/SumScalarLEVar.h"
#include "constraints/primitives/xTimesyEQz.h"
#include "extensions/BinaryExtension.h"
#include "extensions/MDDExtension.h"
#include "extensions/STRNeg.h"
#include "extensions/ShortSTR2.h"
#include "extensions/Unary.h"
#include "genericFiltering/AC3rm.h"
#include "globals/comparison/AllDifferentBC.h"
#include "globals/comparison/DistinctVectors.h"
#include "globals/comparison/Lexicographic.h"
#include "globals/comparison/NotAllEqual.h"
#include "globals/connection/element/ElementConstant.h"
#include "globals/connection/element/ElementVariable.h"
#include "globals/connection/maximum/MaximumConstantGE.h"
#include "globals/connection/maximum/MaximumConstantLE.h"
#include "globals/connection/minimum/MinimumConstantGE.h"
#include "globals/connection/minimum/MinimumConstantLE.h"
#include "globals/connection/minimum/MinimumVariableEQ.h"
#include "globals/counting/AtLeastK.h"
#include "globals/counting/AtMostK.h"
#include "globals/counting/ExactlyK.h"
#include "globals/counting/ExactlyKVariable.h"
#include "globals/counting/NValuesLEK.h"
#include "globals/summing/Sum.h"
#include "globals/summing/SumEQ.h"
#include "globals/summing/SumGE.h"
#include "globals/summing/SumNE.h"
#include "intension/Intension.h"
#include "mtl/Vec.h"
#include "primitives/DiffXY.h"
#include "primitives/Disjunctive.h"
#include "primitives/DistXYeqZ.h"
#include "primitives/EQ.h"
#include "primitives/GEUnary.h"
#include "primitives/LE.h"
#include "primitives/LEUnary.h"
#include "primitives/LT.h"
#include "primitives/xEqOryk.h"
#include "utils/Verbose.h"

using namespace Cosoco;
/// using namespace FactoryConstraints;

Verbose verbose;

//--------------------------------------------------------------------------------------
// Basic constraints
//--------------------------------------------------------------------------------------

Constraint *FactoryConstraints::newExtensionConstraint(Problem *p, std::string name, vec<Variable *> &vars, vec<vec<int>> &tuples,
                                                       bool isSupport, bool hasStar) {
    Extension *ctr = nullptr;

    if(false && tuples.size() == 1 && isSupport == false) {
        p->addConstraint(new NoGood(*p, name, vars, tuples[0]));
        return nullptr;
    }
    if(vars.size() == 2) {
        int max_size = vars[0]->size() > vars[1]->size() ? vars[0]->size() : vars[1]->size();

        if(isSupport && max_size > options::intConstants["large_bin_extension"])
            ctr = new BinaryExtensionSupport(*p, name, isSupport, vars[0], vars[1]);
        else
            ctr = new BinaryExtension(*p, name, isSupport, vars[0], vars[1]);

    } else {
        if(isSupport) {
            // ctr = new CompactTable(*p, name, vars, tuples.size());
            if(tuples.size() < options::intConstants["smallNbTuples"])
                ctr = new STR0(*p, name, vars, tuples.size());
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


void FactoryConstraints::createConstraintExtensionAs(Problem *p, std::string name, vec<Variable *> &vars, Constraint *c) {
    Extension *ctr            = nullptr;
    auto      *sameConstraint = (Extension *)c;
    assert(sameConstraint->scope.size() == vars.size());
    p->nbExtensionsSharded++;
    if(vars.size() == 1) {
        p->addConstraint(new Unary(*p, name, vars[0], ((Unary *)p->constraints.last())->values,
                                   ((Unary *)p->constraints.last())->areSupports));
        return;
    }
    auto *noGood = dynamic_cast<NoGood *>(c);
    if(noGood != nullptr) {
        p->addConstraint(new NoGood(*p, name, vars, noGood->tuple));
        return;
    }
    if(vars.size() == 2) {
        int max_size = vars[0]->size() > vars[1]->size() ? vars[0]->size() : vars[1]->size();
        if(sameConstraint->isSupport && max_size > options::intConstants["large_bin_extension"])
            ctr = new BinaryExtensionSupport(*p, name, sameConstraint->isSupport, vars[0], vars[1],
                                             (BinaryExtensionSupport *)sameConstraint);
        else
            ctr = new BinaryExtension(*p, name, sameConstraint->isSupport, vars[0], vars[1], (BinaryExtension *)sameConstraint);
    }
    if(vars.size() > 2) {
        if(sameConstraint->isSupport) {
            if(sameConstraint->nbTuples() < options::intConstants["smallNbTuples"])
                ctr = new STR0(*p, name, vars, sameConstraint->tuples);
            else
                ctr = new ShortSTR2(*p, name, vars, sameConstraint->tuples);
        } else
            ctr = new STRNeg(*p, name, vars, sameConstraint->tuples);
    }
    p->addConstraint(ctr);
}

void FactoryConstraints::createConstraintExtension(Problem *p, std::string name, vec<Variable *> &vars, vec<vec<int>> &tuples,
                                                   bool isSupport, bool hasStar) {
    Constraint *c = newExtensionConstraint(p, name, vars, tuples, isSupport, hasStar);
    if(c == nullptr)
        return;
    p->addConstraint(c);
}


void FactoryConstraints::createConstraintXeqAndY(Problem *p, std::string name, Variable *x, vec<Variable *> &l) {
    l.push(x);
    p->addConstraint(new XeqAndY(*p, name, l));
}

void FactoryConstraints::createConstraintXeqOrYeqK(Problem *p, std::string name, Variable *res, vec<Variable *> &cl,
                                                   vec<int> &vals) {
    p->addConstraint(new xEqOryk(*p, name, res, cl, vals));
}

void FactoryConstraints::createConstraintIntension(Problem *p, std::string name, XCSP3Core::Tree *tree, vec<Variable *> &scope) {
    p->addConstraint(new AdapterAC3rm(new Intension(*p, name, tree, scope)));
}


void FactoryConstraints::createConstraintLessThan(Problem *p, std::string name, Variable *x, int k, Variable *y, bool strict) {
    if(strict)
        p->addConstraint(new Lt(*p, name, x, y, k));
    else
        p->addConstraint(new Le(*p, name, x, y, k));
}


void FactoryConstraints::createConstraintXeqYplusk(Problem *p, std::string name, Variable *x, Variable *y, int k) {
    p->addConstraint(new Cosoco::EQ(*p, name, x, y, k));
}


void FactoryConstraints::createConstraintDistXYeqZ(Problem *p, std::string name, Variable *x, Variable *y, Variable *z) {
    p->addConstraint(new DistXYeqZ(*p, name, x, y, z));
}

void FactoryConstraints::createConstraintXeqYeqK(Problem *p, std::string name, Variable *x, Variable *y, int k) {
    p->addConstraint(new XeqYeqK(*p, name, x, y, k));
}

void FactoryConstraints::createConstraintXeqYneK(Problem *p, std::string name, Variable *x, Variable *y, int k) {
    p->addConstraint(new XeqYneK(*p, name, x, y, k));
}

void FactoryConstraints::createConstraintXeqKleY(Problem *p, std::string name, Variable *x, Variable *y, int k) {
    p->addConstraint(new XeqKleY(*p, name, x, y, k));
}

void FactoryConstraints::createConstraintXeqYleK(Problem *p, std::string name, Variable *x, Variable *y, int k) {
    p->addConstraint(new XeqYleK(*p, name, x, y, k));
}

void FactoryConstraints::createReification(Problem *p, std::string name, Variable *x, Variable *y, Variable *z,
                                           ExpressionType op) {
    assert(x != y && x != z && y != z);
    if(op == OLE) {
        p->addConstraint(new ReifLE(*p, name, x, y, z));
        return;
    }
    if(op == OLT) {
        p->addConstraint(new ReifLT(*p, name, x, y, z));
        return;
    }
    if(op == OEQ) {
        p->addConstraint(new ReifEQ(*p, name, x, y, z));
        return;
    }
    if(op == ONE) {
        p->addConstraint(new ReifNE(*p, name, x, y, z));
        return;
    }
    assert(false);
}

void FactoryConstraints::createConstraintMult(Problem *p, std::string name, Variable *x, Variable *y, Variable *z) {
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
            FactoryConstraints::createConstraintExtension(p, name, scp, tuples, true);
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
        FactoryConstraints::createConstraintExtension(p, name, scp, tuples, true);


        return;
        printf("sdsd\n");
        scp.push(x);
        if(x != y)
            scp.push(y);
        if(x != z && y != z)
            scp.push(z);
        FactoryConstraints::createConstraintIntension(
            p, name, new Tree("eq(" + z->_name + ",mul(" + x->_name + "," + y->_name + "))"), scp);
    } else
        p->addConstraint(new xTimesyEQz(*p, name, x, y, z));
}


void FactoryConstraints::createConstraintUnary(Problem *p, std::string name, Variable *x, vec<int> &values, bool isSupport) {
    p->addConstraint(new Unary(*p, name, x, values, isSupport));
}


void FactoryConstraints::createConstraintUnaryGE(Problem *p, std::string name, Variable *x, int k) {
    p->addConstraint(new GEUnary(*p, name, x, k));
}


void FactoryConstraints::createConstraintUnaryLE(Problem *p, std::string name, Variable *x, int k) {
    p->addConstraint(new LEUnary(*p, name, x, k));
}
//--------------------------------------------------------------------------------------
// Language constraints
//--------------------------------------------------------------------------------------

void FactoryConstraints::createConstraintMDD(Problem *p, std::string name, vec<Variable *> &vars,
                                             vec<XTransition *> &transitions) {
    p->addConstraint(new MDDExtension(*p, name, vars, transitions));
}


void FactoryConstraints::createConstraintMDD(Problem *p, std::string name, vec<Variable *> &vars, Cosoco::MDD *mdd) {
    p->addConstraint(new MDDExtension(*p, name, vars, mdd));
}


void FactoryConstraints::createConstraintRegular(Problem *p, std::string name, vec<Variable *> &vars, string start,
                                                 std::vector<string> &final, vec<XTransition *> &transitions) {
    p->addConstraint(new MDDExtension(*p, name, vars, MDD::buildFromAutomata(name, vars, start, final, transitions)));
}

//--------------------------------------------------------------------------------------
// Circuit constraints
//--------------------------------------------------------------------------------------

void FactoryConstraints::createConstraintCircuit(Problem *p, std::string name, vec<Variable *> &vars) {
    p->addConstraint(new Circuit(*p, name, vars));
}


//--------------------------------------------------------------------------------------
// Comparison constraints
//--------------------------------------------------------------------------------------

void FactoryConstraints::createConstraintAllDiff(Problem *p, std::string name, vec<Variable *> &vars) {
    if(vars.size() == 2) {
        p->addConstraint(new DiffXY(*p, name, vars[0], vars[1]));
        return;
    }
    int nb = 0;
    for(Variable *x : vars)
        if(x->size() > 2 * vars.size())
            nb++;

    if(nb * 4 > 3 * vars.size()) {   // if 75% of the domains are larger than two times the arity. See ACE
        p->addConstraint(new AllDifferentWeak(*p, name, vars));
        return;
    }

    p->addConstraint(new AllDifferentAC(*p, name, vars));
}


void FactoryConstraints::createConstraintAllDiffExcept(Problem *p, std::string name, vec<Variable *> &vars, vector<int> &except) {
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
            FactoryConstraints::createConstraintExtension(p, name, varsInConstraint, tuples, false, false);
        }
    }
}

//----------------------------------------------------------------------


void FactoryConstraints::createExtenstionDistinctVector(Problem *p, std::string name, vec<Variable *> &list1,
                                                        vec<Variable *> &list2) {
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
    FactoryConstraints::createConstraintExtension(p, name, scope, tuples, true, true);
}


void FactoryConstraints::createConstraintAllDiffList(Problem *p, std::string name, vec<vec<Variable *>> &lists) {
    if(0) {
        verbose.log(NORMAL, "c AllDiff List constraint using %d DistinctVectors constraints \n",
                    lists.size() * (lists.size() - 1) / 2);

        for(int i = 0; i < lists.size(); i++)
            for(int j = i + 1; j < lists.size(); j++) p->addConstraint(new DistinctVectors(*p, name, lists[i], lists[j]));
    } else {
        verbose.log(NORMAL, "c AllDiff List constraint using extension constraint\n", lists.size() * (lists.size() - 1) / 2);
        for(int i = 0; i < lists.size(); i++)
            for(int j = i + 1; j < lists.size(); j++)
                FactoryConstraints::createExtenstionDistinctVector(p, name, lists[i], lists[j]);
    }
}


//----------------------------------------------------------------------


void FactoryConstraints::createConstraintAllEqual(Problem *p, std::string name, vec<Variable *> &vars) {
    for(int i = 0; i < vars.size() - 1; i++) p->addConstraint(new Cosoco::EQ(*p, name, vars[i], vars[i + 1]));
}


void FactoryConstraints::createConstraintNotAllEqual(Problem *p, std::string name, vec<Variable *> &vars) {
    p->addConstraint(new NotAllEqual(*p, name, vars));
}


//--------------------------------------------------------------------------------------
// Summing and counting constraints
//--------------------------------------------------------------------------------------

void FactoryConstraints::createConstraintDiff(Problem *p, std::string name, Variable *x, Variable *y, Variable *z) {
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

void FactoryConstraints::createConstraintSum(Problem *p, std::string name, Variable *x, Variable *y, Variable *z) {   // x+y=z
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


void FactoryConstraints::createConstraintSum(Problem *p, std::string name, vec<Variable *> &vars, vec<Variable *> &coeffs, long l,
                                             OrderType order) {
    if(order == LE) {
        p->addConstraint(new SumScalarLEK(*p, name, vars, coeffs, l));
        return;
    }
    throw runtime_error("Not yet implemented scalar");
}

void FactoryConstraints::createConstraintSum(Problem *p, std::string name, vec<Variable *> &vars, vec<Variable *> &coeffs,
                                             Variable *z, OrderType order) {
    if(order == LE) {
        p->addConstraint(new SumScalarLEVar(*p, name, vars, coeffs, z));
        return;
    }
    throw runtime_error("Not yet implemented scalar");
}

void FactoryConstraints::createConstraintSum(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &coeffs, long l,
                                             OrderType order) {
    Sum *ctr = nullptr;

    // Rearrange coeffs.
    if(order == OrderType::LE || order == OrderType::LT) {
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
        case OrderType::LE:
            ctr = new SumGE(*p, name, vars, coeffs, l);
            break;
        case OrderType::LT:
            ctr = new SumGE(*p, name, vars, coeffs, l + 1);
            break;
        case OrderType::GE:
            ctr = new SumGE(*p, name, vars, coeffs, l);
            break;
        case OrderType::GT:
            ctr = new SumGE(*p, name, vars, coeffs, l + 1);   // TODO
            break;
        case OrderType::IN:
            throw runtime_error("This is forbidden to construct a sum with IN operator");
            break;
        case OrderType::EQ:
            ctr = new SumEQ(*p, name, vars, coeffs, l);
            break;
        case OrderType::NE:
            ctr = new SumNE(*p, name, vars, coeffs, l);
            break;
    }
    assert(ctr != nullptr);
    p->addConstraint(ctr);
}


void FactoryConstraints::createConstraintAtLeast(Problem *p, std::string name, vec<Variable *> &vars, int value, int k) {
    if(k == 0)
        return;
    p->addConstraint(new AtLeastK(*p, name, vars, k, value));
}


void FactoryConstraints::createConstraintAtMost(Problem *p, std::string name, vec<Variable *> &vars, int value, int k) {
    p->addConstraint(new AtMostK(*p, name, vars, k, value));
}

void FactoryConstraints::createConstraintAmong(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &values, int k) {
    p->addConstraint(new Among(*p, name, vars, values, k));
}


void FactoryConstraints::createConstraintExactly(Problem *p, std::string name, vec<Variable *> &vars, int value, int k) {
    p->addConstraint(new ExactlyK(*p, name, vars, k, value));
}


void FactoryConstraints::createConstraintExactlyVariable(Problem *p, std::string name, vec<Variable *> &vars, int value,
                                                         Variable *k) {
    p->addConstraint(new ExactlyKVariable(*p, name, vars, k, value));
}


void FactoryConstraints::createConstraintNValuesLE(Problem *p, std::string name, vec<Variable *> &vars, int k) {
    p->addConstraint(new NValuesLEK(*p, name, vars, k));
}


void FactoryConstraints::createConstraintNValuesEQV(Problem *p, std::string name, vec<Variable *> &vars, Variable *k) {
    p->addConstraint(new NValuesEQVar(*p, name, vars, k));
}


//--------------------------------------------------------------------------------------
// Connection constraints
//--------------------------------------------------------------------------------------
void FactoryConstraints::createConstraintElementConstant(Problem *p, std::string name, vec<Variable *> &vars, Variable *index,
                                                         int startIndex, int v) {
    p->addConstraint(new ElementConstant(*p, name, vars, index, v, startIndex == 1));
}


void FactoryConstraints::createConstraintElementVariable(Problem *p, std::string name, vec<Variable *> &vars, Variable *index,
                                                         int startIndex, Variable *v) {
    p->addConstraint(new ElementVariable(*p, name, vars, index, v, startIndex == 1));
}


void FactoryConstraints::createConstraintElementMatrix(Problem *p, std::string name, vec<vec<Variable *>> &matrix,
                                                       Variable *rindex, Variable *cindex, int value) {
    p->addConstraint(new ElementMatrix(*p, name, matrix, rindex, cindex, value));
}


void FactoryConstraints::createConstraintCardinality(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &values,
                                                     vec<Occurs> &occurs) {
    verbose.log(DEBUGVERBOSE, "c Create Cardinality(vars,values,occurs) constraint using %d Exactly/AtMost.. constraint \nc\n",
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
            FactoryConstraints::createConstraintExactly(p, name + "card as exactly", vars, values[i], occurs[i].value);
        if(occurs[i].type == OCCURS_INTERVAL) {
            FactoryConstraints::createConstraintAtLeast(p, name + "card as atleast", vars, values[i], occurs[i].min);
            FactoryConstraints::createConstraintAtMost(p, name + "card as atleast", vars, values[i], occurs[i].max);
        }
        if(occurs[i].type == OCCURS_VARIABLE) {
            FactoryConstraints::createConstraintExactlyVariable(p, name + "card as exactly", vars, values[i], occurs[i].x);
        }
    }
}


void FactoryConstraints::createConstraintOrdered(Problem *p, std::string name, vec<Variable *> &vars, vector<int> &lengths,
                                                 OrderType op) {
    for(int i = 0; i < vars.size() - 1; i++) {
        int k = lengths.size() == 0 ? 0 : lengths[i];
        if(op == OrderType::LE)
            p->addConstraint(new Le(*p, name + vars[i]->name() + " op " + name + vars[i + 1]->name(), vars[i], vars[i + 1], k));
        if(op == OrderType::LT)
            p->addConstraint(new Lt(*p, name + vars[i]->name() + " op " + name + vars[i + 1]->name(), vars[i], vars[i + 1], k));

        if(op == OrderType::GE)
            p->addConstraint(new Le(*p, name + vars[i]->name() + " op " + name + vars[i + 1]->name(), vars[i + 1], vars[i], -k));
        if(op == OrderType::GT)
            p->addConstraint(new Lt(*p, name + vars[i]->name() + " op " + name + vars[i + 1]->name(), vars[i + 1], vars[i], -k));
    }
}

void FactoryConstraints::createConstraintOrdered(Problem *p, std::string name, vec<Variable *> &vars, vec<Variable *> &lengths,
                                                 OrderType op) {
    for(int i = 0; i < vars.size() - 1; i++) {
        vec<int> coeffs;
        coeffs.push(1);
        coeffs.push(1);
        coeffs.push(-1);
        vec<Variable *> tmp;
        tmp.push(vars[i]);
        tmp.push(lengths[i]);
        tmp.push(vars[i + 1]);
        tmp[0] = vars[i];
        tmp[1] = lengths[i];
        tmp[2] = vars[i + 1];
        createConstraintSum(p, name, tmp, coeffs, 0, op);
    }
}


void FactoryConstraints::createConstraintLex(Problem *p, const std::string &name, vec<Variable *> &vars1, vec<Variable *> &vars2,
                                             OrderType op) {
    if(op == OrderType::LE)
        p->addConstraint(new Lexicographic(*p, name, vars1, vars2, false));
    if(op == OrderType::LT)
        p->addConstraint(new Lexicographic(*p, name, vars1, vars2, true));

    if(op == OrderType::GE)
        p->addConstraint(new Lexicographic(*p, name, vars2, vars1, false));
    if(op == OrderType::GT)
        p->addConstraint(new Lexicographic(*p, name, vars2, vars1, true));
}


void FactoryConstraints::createContraintChannelXY(Problem *p, std::string name, vec<Variable *> &X, vec<Variable *> &Y,
                                                  int startX, int startY) {
    verbose.log(NORMAL, "c Create channel(X,Y) constraint using %d element constraint \n", X.size());
    for(int i = 0; i < X.size(); i++) FactoryConstraints::createConstraintElementConstant(p, name, Y, X[i], startY, i);
}

void FactoryConstraints::createConstraintChannel(Problem *p, string name, vec<Variable *> &vars, int index) {
    assert(index == 0);
    FactoryConstraints::createConstraintAllDiff(p, name, vars);
    for(int i = 0; i < vars.size(); i++) {
        FactoryConstraints::createConstraintElementConstant(p, name, vars, vars[i], 0, i);
    }
}

//-----------------------------------------------------------------------

void FactoryConstraints::createConstraintDisjunctive(Problem *p, std::string name, Variable *x1, Variable *x2, int w1, int w2,
                                                     Variable *aux) {
    p->addConstraint(new Disjunctive(*p, name, x1, x2, w1, w2, aux));
    // string tmp = "or(le(add(" + x1->_name + "," + to_string(w1) + ")," + x2->_name + "),le(add(" + x2->_name + "," +
    // to_string(w2) + ")," + x1->_name + "))"; createConstraintIntension(p, "no overlap", tmp);
}

void FactoryConstraints::createConstraintDisjunctiveVars(Problem *p, std::string name, Variable *x1, Variable *x2, Variable *w1,
                                                         Variable *w2) {
    p->addConstraint(new DisjunctiveVars(*p, name, x1, x2, w1, w2));
}

void FactoryConstraints::createConstraintDisjunctive2D(Problem *p, std::string name, Variable *x1, Variable *x2, Variable *y1,
                                                       Variable *y2, int w1, int w2, int h1, int h2, Variable *z) {
    p->addConstraint(new Disjunctive2D(*p, name, x1, x2, y1, y2, w1, w2, h1, h2, z));
}

void FactoryConstraints::createConstraintDisjunctive2DVar(Problem *p, std::string name, Variable *x1, Variable *x2, Variable *y1,
                                                          Variable *y2, Variable *w1, Variable *w2, Variable *h1, Variable *h2,
                                                          Variable *z) {
    p->addConstraint(new Disjunctive2DVar(*p, name, x1, x2, y1, y2, w1, w2, h1, h2, z));
}
//
//-----------------------------------------------------------------------

void FactoryConstraints::createConstraintMaximumLE(Problem *p, std::string name, vec<Variable *> &vars, int k) {
    p->addConstraint(new MaximumConstantLE(*p, name, vars, k));
}


void FactoryConstraints::createConstraintMaximumGE(Problem *p, std::string name, vec<Variable *> &vars, int k) {
    p->addConstraint(new MaximumConstantGE(*p, name, vars, k));
}


void FactoryConstraints::createConstraintMinimumLE(Problem *p, std::string name, vec<Variable *> &vars, int k) {
    p->addConstraint(new MinimumConstantLE(*p, name, vars, k));
}


void FactoryConstraints::createConstraintMinimumGE(Problem *p, std::string name, vec<Variable *> &vars, int k) {
    p->addConstraint(new MinimumConstantGE(*p, name, vars, k));
}


void FactoryConstraints::createConstraintMinimumVariableEQ(Problem *p, std::string name, vec<Variable *> &vars, Variable *value) {
    if(vars.contains(value)) {
        verbose.log(NORMAL, "c min(x...)=x -> use inequalities\n");
        for(Variable *x : vars) {
            if(x != value)
                p->addConstraint(new Le(*p, name, value, x, 0));
        }
    } else
        p->addConstraint(new MinimumVariableEQ(*p, name, vars, value));
}


void FactoryConstraints::createConstraintMaximumVariableEQ(Problem *p, std::string name, vec<Variable *> &vars, Variable *value) {
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
void FactoryConstraints::createConstraintCumulative(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &lengths,
                                                    vec<int> &heights, Variable *limit) {
    p->addConstraint(new CumulativeVariablesC(*p, name, vars, lengths, heights, limit));
}

void FactoryConstraints::createConstraintCumulative(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &lengths,
                                                    vec<int> &heights, int limit) {
    p->addConstraint(new Cumulative(*p, name, vars, vars, lengths, heights, limit));
}


void FactoryConstraints::createConstraintCumulativeHeightVariable(Problem *p, std::string name, vec<Variable *> &vars,
                                                                  vec<int> &lengths, vec<Variable *> &heights, int limit) {
    p->addConstraint(new CumulativeVariablesH(*p, name, vars, lengths, heights, limit));
}

void FactoryConstraints::createConstraintCumulativeHeightVariableLV(Problem *p, std::string name, vec<Variable *> &vars,
                                                                    vec<int> &lengths, vec<Variable *> &heights,
                                                                    Variable *limit) {
    p->addConstraint(new CumulativeVariablesHLimitV(*p, name, vars, lengths, heights, limit));
}

void FactoryConstraints::createConstraintCumulativeWidthVariables(Problem *p, std::string name, vec<Variable *> &vars,
                                                                  vec<Variable *> &lengths, vec<int> &heights, int limit) {
    p->addConstraint(new CumulativeVariablesW(*p, name, vars, lengths, heights, limit));
}

void FactoryConstraints::createConstraintCumulativeHeightAndWidthVariables(Problem *p, std::string name, vec<Variable *> &vars,
                                                                           vec<Variable *> &widths, vec<Variable *> &heights,
                                                                           int limit) {
    p->addConstraint(new CumulativeVariablesHW(*p, name, vars, widths, heights, limit));
}

void FactoryConstraints::createConstraintCumulativeHeightAndWidthAndConditionVariables(
    Problem *p, std::string name, vec<Variable *> &vars, vec<Variable *> &widths, vec<Variable *> &heights, Variable *limit) {
    p->addConstraint(new CumulativeVariablesHWC(*p, name, vars, widths, heights, limit));
}

void FactoryConstraints::createConstraintNoOverlap(Problem *p, std::string name, vec<Variable *> &X, vec<int> &width,
                                                   vec<Variable *> &Y, vec<int> &heights) {
    p->addConstraint(new NoOverlap(*p, name, X, width, Y, heights));
}


void FactoryConstraints::createConstraintPrecedence(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &values,
                                                    bool covered) {
    p->addConstraint(new Precedence(*p, name, vars, values, covered));
}

void FactoryConstraints::createConstraintBinPacking(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &sizes,
                                                    vec<int> &limits) {
    p->addConstraint(new BinPacking(*p, name, vars, sizes, limits));
}

void FactoryConstraints::createConstraintBinPacking(Problem *p, std::string name, vec<Variable *> &vars, vec<int> &sizes,
                                                    vec<Variable *> &loads) {
    p->addConstraint(new BinPackingLoad(*p, name, vars, sizes, loads));
}
