#pragma once
#ifndef COSOCO_TEST_UTILS
#define COSOCO_TEST_UTILS

#include "solver/utils/Options.h"

namespace Cosoco {

/**
 * The purpose of this class it to have solver options initialized with
 * their default values and having all options automatically cleared when
 * the object gets deleted
 */
class OptionDefaultAndCleaner {
   public:
    OptionDefaultAndCleaner() { options::createOptions(); }
    ~OptionDefaultAndCleaner() {
        options::stringOptions.clear();
        options::boolOptions.clear();
        options::intOptions.clear();
        options::doubleOptions.clear();
    }
};

}   // namespace Cosoco

#endif