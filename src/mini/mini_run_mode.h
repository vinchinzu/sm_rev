#ifndef SM_MINI_RUN_MODE_H_
#define SM_MINI_RUN_MODE_H_

#include "types.h"

typedef enum MiniRunMode {
  kMiniRunMode_LandingSite = 0,
  kMiniRunMode_ClimbEndless = 1,
  kMiniRunMode_ClimbOriginal = 2,
} MiniRunMode;

void MiniRunMode_Set(MiniRunMode mode);
MiniRunMode MiniRunMode_Get(void);
bool MiniRunMode_IsClimbRoom(void);
bool MiniRunMode_IsClimbEndless(void);
bool MiniRunMode_IsClimbOriginal(void);

#endif  // SM_MINI_RUN_MODE_H_
