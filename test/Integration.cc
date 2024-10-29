#include <gtest/gtest.h>

#include "constraints/primitives/GEUnary.h"
#include "core/Problem.h"
#include "core/Variable.h"
#include "core/domain/DomainRange.h"
#include "solver/Solver.h"
#include "solver/restarts/Restart.h"
#include "test/Utils.h"

TEST(IntegrationTest, oneVarNoConstraint) {
    Cosoco::OptionDefaultAndCleaner _defaultOptions;

    Cosoco::Problem     p("oneVarNoConstraint");
    Cosoco::DomainRange domain(0, 4);
    p.createVariable(std::string("MyOnlyVariable"), domain);
    Cosoco::Solver                       solver(p);
    Cosoco::vec<Cosoco::RootPropagation> assumps;
    p.delayedConstruction();
    try {
        auto result = solver.solve(assumps);
        EXPECT_EQ(result, R_SAT);
    } catch(std::exception &e) {
        printf("%s", e.what());
        FAIL();
    }
}

TEST(IntegrationTest, oneVarOneConstraint) {
    Cosoco::OptionDefaultAndCleaner _defaultOptions;

    Cosoco::Problem     p("oneVarNoConstraint");
    Cosoco::DomainRange domain(0, 4);
    auto               *var = p.createVariable(std::string("MyOnlyVariable"), domain);
    p.addConstraint(std::make_unique<Cosoco::GEUnary>(p, "MyOnlyConstraint", var, 3));
    Cosoco::Solver                       solver(p);
    Cosoco::vec<Cosoco::RootPropagation> assumps;
    p.delayedConstruction();
    try {
        auto result = solver.solve(assumps);
        EXPECT_EQ(result, R_SAT);
    } catch(std::exception &e) {
        printf("%s", e.what());
        FAIL();
    }
}