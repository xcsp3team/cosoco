//
// Created by audemard on 03/04/24.
//
#include "DomainRange.h"
#include "DomainValues.h"

using namespace Cosoco;

bool DomainRange::equals(Cosoco::Domain *d) {
    auto *dr = dynamic_cast<DomainRange *>(d);
    if(dr == nullptr)
        return false;
    return min == dr->min && max == dr->max;
}

bool DomainValue::equals(Domain *d) {
    auto *dv = dynamic_cast<DomainValue *>(d);
    if(dv == nullptr || values.size() != dv->values.size())
        return false;
    for(int i = 0; i < values.size(); i++)
        if(values[i] != dv->values[i])
            return false;
    return true;
}