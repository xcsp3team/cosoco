#pragma once
#ifndef COSOCO_CONSTANTS
#define COSOCO_CONSTANTS

#ifdef USE_XCSP3
#include "XCSP3Constants.h"
#else
#include <climits>
#define STAR INT_MAX
#endif

#endif