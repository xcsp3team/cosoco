#include "Unary.h"

#include "solver/Solver.h"


using namespace Cosoco;
//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------

bool Cosoco::Unary::isSatisfiedBy(Cosoco::vec<int> &tuple) { return values.contains(tuple[0]) == areSupports; }


//----------------------------------------------------------
// Construction and initialisation
//----------------------------------------------------------

Unary::Unary(Problem &p, std::string n, Variable *xx, const vec<int> &vals, bool areS)
    : Constraint(p, n, createScopeVec(xx)), x(xx), done(false), areSupports(areS) {
    vals.copyTo(values);
    type = "Unary";
}


Cosoco::State Cosoco::Unary::status() { return UNDEF; }


void Cosoco::Unary::reinitialize() { }

//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool Cosoco::Unary::filter(Cosoco::Variable *dummy) {
    if(done) {
        // If new call => because restarts and entail is deleted...
        solver->entail(this);
        return true;
    }
    done = true;

    if(areSupports) {
        if(values.size() == 1) {
            bool ret = solver->assignToVal(x, values[0]);
            solver->entail(this);
            return ret;
        }

        for(int idv : x->domain) {
            int v = x->domain.toVal(idv);
            if(values.contains(v) == false && solver->delVal(x, v) == false)
                return false;
        }
        solver->entail(this);
        return true;
    }

    for(int v : values)
        if(solver->delVal(x, v) == false)
            return false;
    solver->entail(this);
    return true;
}
