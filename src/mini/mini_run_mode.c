#include "mini_run_mode.h"

static MiniRunMode g_run_mode = kMiniRunMode_LandingSite;

void MiniRunMode_Set(MiniRunMode mode) {
  g_run_mode = mode;
}

MiniRunMode MiniRunMode_Get(void) {
  return g_run_mode;
}

bool MiniRunMode_IsClimbRoom(void) {
  return g_run_mode == kMiniRunMode_ClimbEndless ||
         g_run_mode == kMiniRunMode_ClimbOriginal;
}

bool MiniRunMode_IsClimbEndless(void) {
  return g_run_mode == kMiniRunMode_ClimbEndless;
}

bool MiniRunMode_IsClimbOriginal(void) {
  return g_run_mode == kMiniRunMode_ClimbOriginal;
}
