

#ifndef CARDINALITY_H
#define CARDINALITY_H
#include "constraints/globals/GlobalConstraint.h"


namespace Cosoco {
class Cardinality : public GlobalConstraint {
    vec<int> values;
    vec<int> minOccs;
    vec<int> maxOccs;
    //MatcherCardinality *matcher;

public :
    Cardinality(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &v, vec<int> &min, vec<int> &max); // Min and max occurences
    Cardinality(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &v, vec<int> &nOccs); // fixed nb occs.
    Cardinality(Problem &p, std::string n, vec<Variable *> &vars, vec<int> &v, int min, int max); // Same nb of occs

    // Filtering method, return false if a conflict occurs
    bool filter(Variable *x) override;
    // Checking
    bool isSatisfiedBy(vec<int> &tuple) override;
    bool isCorrectlyDefined() override;



};
}   // namespace Cosoco


#endif   // CARDINALITY_H
