#include <gtest/gtest.h>

#include "core/Problem.h"
#include "core/Variable.h"
#include "core/domain/DomainRange.h"
#include "solver/Solver.h"
#include "solver/restarts/Restart.h"
#include "solver/utils/Options.h"

TEST(IntegrationTest, oneVarNoConstraint) {
    Cosoco::options::createOptions();

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