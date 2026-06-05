#include "mini_climb_mode.h"

#include "mini_climb_endless.h"
#include "mini_editor_camera.h"
#include "mini_run_mode.h"

void MiniClimbMode_Tick(MiniGameState *state) {
  switch (MiniRunMode_Get()) {
  case kMiniRunMode_ClimbEndless:
    MiniClimbEndless_Tick(state);
    return;
  case kMiniRunMode_ClimbOriginal:
    MiniEditorCamera_Follow(state);
    return;
  case kMiniRunMode_LandingSite:
    return;
  }
}
