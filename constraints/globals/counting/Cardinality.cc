//
// Created by audemard on 02/04/25.
//

#include "Cardinality.h"

using namespace Cosoco;
//----------------------------------------------
// Check validity and correct definition
//----------------------------------------------


bool Cardinality::isSatisfiedBy(vec<int> &tuple) {
    for(int i = 0; i < values.size(); i++) {
        int nOccurrences = 0;
        for(int j = 0; j < tuple.size(); j++)
            if(tuple[j] == values[i])
                nOccurrences++;
        if(nOccurrences < minOccs[i] || nOccurrences > maxOccs[i])
            return false;
    }
    return true;
}


bool Cardinality::isCorrectlyDefined() { return true; }


//----------------------------------------------------------
// Filtering
//----------------------------------------------------------

bool Cardinality::filter(Variable *x) {
    if(matcher->findMaximumMatching() == false)
        return false;

    std::cout << "ici\n";
    matcher->removeInconsistentValues();   // no more possible failure at this step
    return true;
}


//----------------------------------------------
// Construction and initialisation
//----------------------------------------------

Cardinality::Cardinality(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &v, vec<int> &min, vec<int> &max)
    : GlobalConstraint(p, n, "Cardinality", vars) {
    min.copyTo(minOccs);
    max.copyTo(maxOccs);
    v.copyTo(values);
    exit(1);
    matcher = new MatcherCardinality(this);
}


Cardinality::Cardinality(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &v, vec<int> &nOccs)
    : GlobalConstraint(p, n, "Cardinality", vars) {
    nOccs.copyTo(minOccs);
    nOccs.copyTo(maxOccs);
    v.copyTo(values);
    matcher = new MatcherCardinality(this);
}


Cardinality::Cardinality(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &v, int min, int max)
    : GlobalConstraint(p, n, "Cardinality", vars) {
    v.copyTo(values);
    minOccs.growTo(v.size(), min);
    maxOccs.growTo(v.size(), max);
    exit(1);
    matcher = new MatcherCardinality(this);
}
