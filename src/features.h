// This file declares build-variant feature flags for sm_rev.
#ifndef SM_FEATURES_H_
#define SM_FEATURES_H_

#include "types.h"

enum {
  BUILD_FULL = 1,
  BUILD_MINI = 2,
  BUILD_MODDABLE = 3,
};

#ifndef CURRENT_BUILD
#define CURRENT_BUILD BUILD_FULL
#endif

#define BUILD_IS_FULL (CURRENT_BUILD == BUILD_FULL)
#define BUILD_IS_MINI (CURRENT_BUILD == BUILD_MINI)
#define BUILD_IS_MODDABLE (CURRENT_BUILD == BUILD_MODDABLE)

#if CURRENT_BUILD == BUILD_MINI || CURRENT_BUILD == BUILD_MODDABLE
// Mini-family builds link shared game systems and constrain content/runtime
// entry to the Landing Site slice instead of compiling broad systems out.
#define MINI_LANDING_SITE_ONLY 1
#define NO_SOUND 1
#endif

#endif  // SM_FEATURES_H_
