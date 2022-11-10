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
    : Constraint(p, n, xx), x(xx), done(false), areSupports(areS) {
    vals.copyTo(values);
    type = "Unary";
}


Cosoco::State Cosoco::Unary::status() { return  done ? CONSISTENT : UNDEF; }


void Cosoco::Unary::reinitialize() { done = false; }

//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool Cosoco::Unary::filter(Cosoco::Variable *dummy) {
    done = true;

    if(areSupports) {
        if(values.size() == 1)
            return solver->assignToVal(x, values[0]);


        for(int idv : reverse(x->domain)) {
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
